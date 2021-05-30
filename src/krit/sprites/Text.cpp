#include "krit/sprites/Text.h"
#include "krit/utils/Slice.h"
#include "harfbuzz/hb.h"
#include <cassert>
#include <stack>

namespace krit {

enum TextOpcodeType: int {
    SetColor,
    SetAlign,
    SetCustom,
    PopColor,
    PopAlign,
    PopCustom,
    GlyphBlock,
    NewLine,
    RenderSprite,
    Tab,
};

std::unordered_map<std::string, TextFormatTagOptions> Text::formatTags = {
    {"\n", TextFormatTagOptions().setNewline()},
    {"br", TextFormatTagOptions().setNewline()},
    {"left", TextFormatTagOptions().setAlign(LeftAlign)},
    {"center", TextFormatTagOptions().setAlign(CenterAlign)},
    {"right", TextFormatTagOptions().setAlign(RightAlign)},
    {"\t", TextFormatTagOptions().setTab()},
    {"tab", TextFormatTagOptions().setTab()},
};

void Text::addFormatTag(std::string tagName, TextFormatTagOptions tagOptions) {
    Text::formatTags.insert(std::make_pair(tagName, tagOptions));
}

template <typename T> void clearStack(std::stack<T> &stack) {
    while (!stack.empty()) stack.pop();
}

struct GlyphRenderStack {
    std::stack<Color> color;
    std::stack<AlignType> align;
    std::stack<CustomTextRenderFunction*> custom;

    GlyphRenderStack() {}

    void clear() {
        clearStack(this->color);
        clearStack(this->align);
        clearStack(this->custom);
    }
};

struct GlyphSequence {
    size_t startIndex = 0;
    size_t length = 0;
    double trailingWhitespace = 0;
    double horizontalSize = 0;

    void append(const GlyphSequence &other) {
        assert(other.startIndex == startIndex + length);
        length += other.length;
        horizontalSize += trailingWhitespace + other.horizontalSize;
        trailingWhitespace = other.trailingWhitespace;
    }

    bool empty() { return length; }
};

/**
 * Utility structure to parse format tags from a string of text and generate a
 * Vector of TextOpcodes.
 */
struct TextParser {
    static GlyphRenderStack stack;
    static std::vector<TextOpcode> word;

    double lineHeight = 0;
    Point cursor;

    GlyphSequence currentLine;
    GlyphSequence currentWord;
    AlignType currentAlign = LeftAlign;
    int newLineIndex = 0;
    size_t tabIndex = 0;
    
    hb_buffer_t *hbBuf;

    void parseText(Text &txt, std::string &s, bool rich) {
        if (txt.hbBuf) {
            hb_buffer_clear_contents(txt.hbBuf);
        } else {
            txt.hbBuf = hb_buffer_create();
            hb_buffer_set_direction(txt.hbBuf, HB_DIRECTION_LTR);
            hb_buffer_set_script(txt.hbBuf, HB_SCRIPT_LATIN);
            hb_buffer_set_language(txt.hbBuf, hb_language_from_string("en", -1));
        }

        hb_buffer_t *hbBuf = txt.hbBuf;
    
        // find all format tags, and generate a string with the text minus tags
        // FIXME: when adding utf-8 support, iterate over codepoints, not chars
        // vector of (pos, length) tag locations from < to >
        std::vector<std::pair<size_t, size_t>> tagLocations;
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '<') {
                size_t startPos = i;
                for (; i < s.size(); ++i) {
                    if (s[i] == '>') {
                        tagLocations.emplace_back(startPos, i - startPos + 1);
                        break;
                    }
                }
            }
        }
        std::string rawText;
        std::vector<std::pair<size_t, StringSlice>> tags;
        size_t tagNameOffset = 0;
        if (tagLocations.size()) {
            // add text before first tag
            if (tagLocations[0].first > 0) {
                rawText.append(s.c_str(), tagLocations[0].first);
            }
            // alternate tags and any text following them
            for (size_t i = 0; i < tagLocations.size(); ++i) {
                auto &l = tagLocations[i];
                size_t offset = l.first, len = l.second;
                if (s[offset] == '<') {
                    ++offset;
                    --len;
                }
                if (s[offset + len] == '>') {
                    --len;
                }
                if (s[offset + len] == '/') {
                    --len;
                }
                tags.emplace_back(l.first - tagNameOffset, StringSlice(&s[offset], len));
                tagNameOffset += l.second;
                if (l.first + l.second < s.size()) {
                    size_t startPos = l.first + l.second;
                    if (i < tagLocations.size() - 1) {
                        // add text from here to next tag start
                        rawText.append(&s[startPos], tagLocations[i + 1].first - startPos);
                    } else {
                        // add text from here to end
                        rawText.append(&s[startPos], s.size() - startPos);
                    }
                }
            }
        } else {
            rawText = s;
        }

        // use harfbuzz to shape the text
        hb_buffer_add_utf8(hbBuf, rawText.c_str(), rawText.size(), 0, -1);
        txt.font->shape(hbBuf, txt.options.size);
        unsigned int glyphCount;
        hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(hbBuf, &glyphCount);
        hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(hbBuf, &glyphCount);

        // iterate over characters/tags and handle them
        txt.opcodes.clear();
        txt.textDimensions.setTo(0, 0);
        TextParser::word.clear();
        GlyphRenderStack &st = TextParser::stack;
        st.clear();
        st.color.push(txt.baseColor);
        st.align.push(this->currentAlign = txt.options.align);
        st.custom.push(nullptr);
        txt.opcodes.push_back(TextOpcode(NewLine, TextOpcodeData(Dimensions(), txt.options.align)));
        txt.maxChars = rawText.size();

        lineHeight = txt.font->lineHeight(txt.options.size);
        currentLine = GlyphSequence();
        currentWord = GlyphSequence();

        size_t tagPointer = 0;
        for (size_t i = 0; i < glyphCount; ++i) {
            auto &_glyphInfo = glyphInfo[i];
            size_t txtPointer = _glyphInfo.cluster;
            // handle any tags that came before this glyph
            while (tagPointer < tags.size() && tags[tagPointer].first >= txtPointer) {
                this->addTag(txt, tags[tagPointer++].second);
            }
            // handle this glyph
            uint32_t c = rawText[txtPointer];
            switch (c) {
                case '\n': {
                    this->flushWord(txt);
                    this->newLine(txt, true);
                    break;
                }
                default: {
                    if (currentWord.startIndex < 0) {
                        currentWord.startIndex = i;
                    }
                    ++currentWord.length;
                    hb_glyph_position_t _glyphPos = glyphPos[i];
                    int xAdvance = _glyphPos.x_advance;
                    currentWord.horizontalSize += xAdvance;
                    if (c == ' ') {
                        currentWord.trailingWhitespace += xAdvance;
                    } else {
                        currentWord.trailingWhitespace = 0;
                    }
                    if (c == ' ') {
                        this->flushWord(txt);
                    }
                }
            }
        }
        while (tagPointer < tags.size()) {
            this->addTag(txt, tags[tagPointer++].second);
        }
        
        this->flushWord(txt);
        this->newLine(txt, false);
    }

    void addTag(Text &txt, StringSlice tagName) {
        static std::string tagStr;
        bool close = false;
        if (tagName[0] == '/') {
            close = true;
            tagName.setTo(tagName.data + 1, tagName.length - 1);
        }
        tagStr.assign(tagName.data, tagName.length);
        auto found = Text::formatTags.find(tagStr);
        if (found != Text::formatTags.end()) {
            TextFormatTagOptions &tag = found->second;
            if (tag.color.present) {
                if (close) this->addOp(txt, TextOpcode(PopColor));
                else this->addOp(txt, TextOpcode(SetColor, TextOpcodeData(tag.color.value)));
            }
            if (tag.align.present) {
                if (close) this->addOp(txt, TextOpcode(PopAlign));
                else this->addOp(txt, TextOpcode(SetAlign, TextOpcodeData(tag.align.value)));
            }
            if (tag.custom != nullptr) {
                if (close) this->addOp(txt, TextOpcode(PopCustom));
                else this->addOp(txt, TextOpcode(SetCustom, TextOpcodeData(tag.custom)));
            }
            if (tag.sprite) {
                this->addOp(txt, TextOpcode(RenderSprite, TextOpcodeData(tag.sprite)));
            }
            if (tag.newline && !close) {
                this->addOp(txt, TextOpcode(NewLine, TextOpcodeData(Dimensions(), LeftAlign)));
            }
            if (tag.tab && !close) {
                this->addOp(txt, TextOpcode(Tab, TextOpcodeData()));
            }
        }
    }

    void newLine(Text &txt, bool append) {
        if (txt.opcodes[this->newLineIndex].type == NewLine) {
            // std::pair<Dimensions, AlignType> &align = txt.opcodes[this->newLineIndex].data.newLine;
            // update the size of the preceding line
            double add = this->lineHeight + (this->newLineIndex == 0 ? 0 : txt.options.lineSpacing);
            this->cursor.y += this->lineHeight + add;
            txt.opcodes[this->newLineIndex].data.newLine.first.setTo(this->cursor.x, add);
            txt.opcodes[this->newLineIndex].data.newLine.second = this->currentAlign;
        }
        if (append) {
            txt.opcodes.push_back(TextOpcode(
                NewLine,
                TextOpcodeData(Dimensions(), this->currentAlign)
            ));
            this->cursor.x = currentLine.trailingWhitespace = 0;
            this->newLineIndex = txt.opcodes.size() - 1;
        }
        this->tabIndex = 0;
    }

    void flushWord(Text &txt) {
        if (!word.empty()) {
            currentLine.trailingWhitespace = currentWord.trailingWhitespace;
            if (txt.options.wordWrap && cursor.x > 0 && cursor.x - currentLine.trailingWhitespace + currentWord.horizontalSize > txt.dimensions.width()) {
                this->newLine(txt, true);
                this->cursor.x = currentWord.horizontalSize;
            } else {
                this->cursor.x += currentWord.horizontalSize;
            }
            for (TextOpcode &op: TextParser::word) {
                txt.opcodes.push_back(op);
            }
            TextParser::word.clear();
            currentWord.startIndex = -1;
            currentWord.length = 0;
            currentWord.trailingWhitespace = 0;
        }
    }

    void addOp(Text &txt, TextOpcode op) {
        switch (op.type) {
            case SetColor: {
                Color &v = op.data.color;
                TextParser::stack.color.push(v);
                TextParser::word.push_back(op);
                break;
            }
            case PopColor: {
                TextParser::stack.color.pop();
                TextParser::word.push_back(TextOpcode(SetColor, TextOpcodeData(TextParser::stack.color.top())));
                break;
            }
            case SetCustom: {
                CustomTextRenderFunction *f = op.data.custom;
                TextParser::stack.custom.push(f);
                TextParser::word.push_back(op);
                break;
            }
            case PopCustom: {
                TextParser::stack.custom.pop();
                TextParser::word.push_back(TextOpcode(SetCustom, TextOpcodeData(TextParser::stack.custom.top())));
                break;
            }
            case NewLine: {
                this->flushWord(txt);
                this->newLine(txt, true);
                break;
            }
            case SetAlign: {
                AlignType &v = op.data.align;
                this->flushWord(txt);
                if (this->cursor.x > 0) {
                    this->newLine(txt, true);
                }
                TextParser::stack.align.push(this->currentAlign = v);
                break;
            }
            case PopAlign: {
                this->flushWord(txt);
                TextParser::stack.align.pop();
                if (this->cursor.x > 0) {
                    this->newLine(txt, true);
                }
                this->currentAlign = TextParser::stack.align.top();
                break;
            }
            case RenderSprite: {
                auto sprite = op.data.sprite;
                auto size = sprite->getSize();
                double imageWidth = size.width();
                TextParser::word.push_back(op);
                currentWord.trailingWhitespace = 0;
                currentWord.horizontalSize += imageWidth;
                // this->wordHeight = std::max(wordHeight, size.height());
                // this->thisLineHeight = std::max(this->wordHeight, this->thisLineHeight);
                if (cursor.x > txt.dimensions.width()) {
                    txt.dimensions.width() = cursor.x;
                }
                break;
            }
            case Tab: {
                if (this->tabIndex < txt.tabStops.size()) {
                    this->flushWord(txt);
                    cursor.x = txt.tabStops[this->tabIndex];
                    txt.opcodes.push_back(op);
                }
                break;
            }
            default: {
                TextParser::word.push_back(op);
            }
        }
    }
};

Text::~Text() {
    if (hbBuf) {
        hb_buffer_destroy(hbBuf);
    }
}


Text &Text::refresh() {
    if (this->dirty) {
        // TODO: reuse static parser...
        TextParser parser;
        parser.parseText(*this, this->text, this->rich);
        this->dirty = false;
    }
    return *this;
}

void Text::resize(double w, double h) {
    if (this->options.wordWrap) {
        if ((this->dimensions.width() != static_cast<int>(w)) || (this->dimensions.height() != static_cast<int>(h))) {
            this->dirty = true;
            this->dimensions.setTo(w, h);
        }
    } else {
        this->dimensions.setTo(w, h);
    }
}

void Text::render(RenderContext &ctx) {
    this->refresh();

    // Font *font = this->options.font;
    Color color = this->color * this->baseColor;
    double thisLineHeight = 0;
    Point cursor;
    CustomTextRenderFunction *custom = nullptr;
    double totalWidth = this->options.wordWrap ? this->dimensions.width() : this->textDimensions.width();
    int charCount = this->charCount;
    size_t tabIndex = 0;

    for (TextOpcode &op: this->opcodes) {
        switch (op.type) {
            case SetColor: {
                color = this->color * op.data.color;
                break;
            }
            case SetCustom: {
                custom = op.data.custom;
                break;
            }
            case NewLine: {
                tabIndex = 0;
                Dimensions &dims = op.data.newLine.first;
                double align;
                switch (op.data.newLine.second) {
                    case LeftAlign: align = 0; break;
                    case CenterAlign: align = 0.5; break;
                    case RightAlign: align = 1; break;
                }
                cursor.setTo(
                    (totalWidth - dims.width()) * this->scale.x * align,
                    cursor.y + thisLineHeight * this->scale.y
                );
                thisLineHeight = dims.height();
                break;
            }
            case RenderSprite: {
                if (charCount > -1 && --charCount < 0) {
                    return;
                }
                auto sprite = op.data.sprite;
                GlyphRenderData renderData(cursor);
                if (custom) {
                    custom(&ctx, this, &renderData);
                }
                auto size = sprite->getSize();
                sprite->scale.setTo(scale);
                sprite->position.setTo(this->position.x + renderData.position.x, this->position.y + renderData.position.y + (thisLineHeight * scale.y - size.height()));
                Color originalColor(sprite->color);
                sprite->color = sprite->color * this->color;
                sprite->render(ctx);
                sprite->color = originalColor;
                cursor.x += size.width();
                break;
            }
            // case GlyphBlock: {
            //     StringSlice &txt = op.data.rawText;
            //     for (size_t i = 0; i < txt.length; ++i) {
            //         if (charCount > -1 && --charCount < 0) {
            //             return;
            //         }
            //         unsigned char c = txt[i];
            //         GlyphData glyph = font->getGlyph(c);
            //         if (glyph.id && glyph.id == c) {
            //             int xAdvance = glyph.xAdvance;
            //             GlyphRenderData renderData(
            //                 c,
            //                 color,
            //                 scale,
            //                 cursor
            //             );
            //             if (lastId != -1) {
            //                 double kern = font->kern(lastId, glyph.id) * renderData.scale.x * this->scale.x * baseScale;
            //                 if (kern < 0) {
            //                     cursor.x += kern;
            //                     renderData.position.x += kern;
            //                 }
            //             }
            //             if (c == ' ' || c == '\t') {
            //                 // it's whitespace; just advance the cursor and move on
            //                 lastId = -1;
            //             // TODO: escape characters
            //             } else {
            //                 // if (this->options.dynamic && custom) {
            //                 //     custom(&ctx, this, &renderData);
            //                 // }
            //                 // Matrix matrix;
            //                 // matrix
            //                 //     .translate(glyph.offset.x, glyph.offset.y)
            //                 //     .scale(renderData.scale.x * this->scale.x * baseScale, renderData.scale.y * this->scale.y * baseScale)
            //                 //     .translate(this->position.x + renderData.position.x, this->position.y + renderData.position.y);
            //                 // DrawKey key;
            //                 // key.image = font->getPage(glyph.page);
            //                 // key.smooth = this->smooth;
            //                 // key.blend = this->blendMode;
            //                 // ctx.addRect(key, glyph.rect, matrix, renderData.color);
            //             }
            //             cursor.x += xAdvance * renderData.scale.x * this->scale.x * baseScale;
            //             lastId = glyph.id;
            //         } else {
            //             lastId = -1;
            //         }
            //     }
            //     break;
            // }
            case Tab: {
                cursor.x = this->tabStops[tabIndex++];
                break;
            }
            // other opcodes (e.g. the Pop variants) are removed during parsing
            default: {}
        }
    }
}

}
