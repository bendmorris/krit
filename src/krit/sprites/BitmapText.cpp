#include "krit/sprites/BitmapText.h"
#include "krit/render/ImageRegion.h"
#include <algorithm>
#include <stack>
#include <utility>

namespace krit {

std::unordered_map<std::string, FormatTagOptions> BitmapText::formatTags = {
    {"br", FormatTagOptions().setNewline()},
    {"left", FormatTagOptions().setAlign(LeftAlign)},
    {"center", FormatTagOptions().setAlign(CenterAlign)},
    {"right", FormatTagOptions().setAlign(RightAlign)},
    {"tab", FormatTagOptions().setTab()},
};

void BitmapText::addFormatTag(std::string tagName, FormatTagOptions tagOptions) {
    BitmapText::formatTags.insert(std::make_pair(tagName, tagOptions));
}

template <typename T> void clearStack(std::stack<T> &stack) {
    while (!stack.empty()) stack.pop();
}

struct GlyphRenderStack {
    std::stack<BitmapFont*> font;
    // std::stack<int> size;
    std::stack<double> scale;
    std::stack<Color> color;
    std::stack<AlignType> align;
    std::stack<CustomRenderFunction*> custom;

    GlyphRenderStack() {}

    void clear() {
        clearStack(this->font);
        // clearStack(this->size);
        clearStack(this->scale);
        clearStack(this->color);
        clearStack(this->align);
        clearStack(this->custom);
    }
};

/**
 * Utility structure to parse format tags from a string of text and generate a
 * Vector of TextOpcodes.
 */
struct TextParser {
    static GlyphRenderStack stack;
    static std::vector<TextOpcode> word;

    double thisLineHeight = 0;
    Point cursor;
    double trailingWhitespace = 0;
    StringSlice wordSegment;
    double wordSegmentLength = 0;
    double wordSegmentTrailingWhitespace = 0;
    double wordLength = 0;
    double wordTrailingWhitespace = 0;
    double wordHeight = 0;
    double currentScale = 1;
    BitmapFont *currentFont;
    AlignType currentAlign = LeftAlign;
    int newLineIndex = 0;
    size_t tabIndex = 0;

    void parseText(BitmapText &txt, std::string &s, bool rich) {
        txt.opcodes.clear();
        txt.textDimensions.setTo(0, 0);
        TextParser::word.clear();
        GlyphRenderStack &st = TextParser::stack;
        st.clear();
        st.font.push(this->currentFont = txt.options.font.get());
        // st.size.push(txt.options.size);
        st.scale.push(1);
        st.color.push(txt.color);
        st.align.push(this->currentAlign = txt.options.align);
        st.custom.push(nullptr);
        int tagEnd = 0;

        this->wordSegment = StringSlice(&s[0], 0);

        txt.opcodes.push_back(TextOpcode(NewLine, TextOpcodeData(Dimensions(), txt.options.align)));

        while (true) {
            StringSlice tag;
            if (rich) {
                // look for a tag starting at this position
                if (this->wordSegment[this->wordSegment.length] == '<') {
                    this->flushWord(txt);
                    tagEnd = 1;
                    while (this->wordSegment[tagEnd] != 0 && this->wordSegment[tagEnd] != '>') {
                        ++tagEnd;
                    }
                    int tagLength = tagEnd - 1;
                    // treat self-closing tag as open tag
                    if (this->wordSegment[tagEnd - 1] == '/') {
                        --tagLength;
                    }
                    tag.setTo(&this->wordSegment[this->wordSegment.length + 1], tagLength);
                    this->wordSegment.setTo(&this->wordSegment.str[tagEnd + 1], 0);
                }
            }
            if (tag.length > 0) {
                this->addTag(txt, tag);
            } else {
                if (this->wordSegment[this->wordSegment.length] == 0) {
                    break;
                } else {
                    this->wordHeight = std::max(this->wordHeight, this->currentFont->lineHeight * this->currentScale);
                    char c = this->wordSegment[this->wordSegment.length];
                    switch (c) {
                        case '\n': {
                            this->flushWord(txt);
                            this->newLine(txt, true);
                            this->wordSegment.str = &this->wordSegment.str[1];
                            break;
                        }
                        default: {
                            this->wordSegment.length++;
                            GlyphData glyph = this->currentFont->getGlyph(c);
                            if (glyph.id) {
                                int xAdvance = glyph.xAdvance;
                                this->wordSegmentLength += xAdvance * this->currentScale;
                                if (c == ' ') {
                                    this->wordSegmentTrailingWhitespace += xAdvance * this->currentScale;
                                } else {
                                    this->wordSegmentTrailingWhitespace = 0;
                                }
                            }
                            if (c == ' ') {
                                this->flushWord(txt);
                            }
                        }
                    }
                }
            }
        }
        this->flushWord(txt);
        this->newLine(txt, false);

        txt.maxChars = 0;
        for (auto &op : txt.opcodes) {
            switch (op.type) {
                case NewLine: {
                    Dimensions &dims = op.data.newLine.first;
                    txt.textDimensions.x = std::max(txt.textDimensions.x, dims.x);
                    txt.textDimensions.y += dims.y;
                    // fallthrough
                }
                case TextBlock: {
                    txt.maxChars += op.data.text.length;
                    break;
                }
                case RenderSprite:
                    ++txt.maxChars;
                    break;
                default: {}
            }
        }
    }

    void addTag(BitmapText &txt, StringSlice tagName) {
        static std::string tagStr;
        bool close = false;
        if (tagName[0] == '/') {
            close = true;
            tagName.setTo(tagName.str + 1, tagName.length - 1);
        }
        tagStr.assign(tagName.str, tagName.length);
        auto found = BitmapText::formatTags.find(tagStr);
        if (found != BitmapText::formatTags.end()) {
            FormatTagOptions &tag = found->second;
            if (tag.color.present) {
                if (close) this->addOp(txt, TextOpcode(PopColor));
                else this->addOp(txt, TextOpcode(SetColor, TextOpcodeData(tag.color.value)));
            }
            if (tag.scale.present) {
                if (close) this->addOp(txt, TextOpcode(PopScale));
                else this->addOp(txt, TextOpcode(SetScale, TextOpcodeData(tag.scale.value)));
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
            if (tag.font) {
                if (close) this->addOp(txt, TextOpcode(PopFont));
                else this->addOp(txt, TextOpcode(SetFont, TextOpcodeData(tag.font)));
            }
        }
    }

    void newLine(BitmapText &txt, bool append) {
        if (txt.opcodes[this->newLineIndex].type == NewLine) {
            // std::pair<Dimensions, AlignType> &align = txt.opcodes[this->newLineIndex].data.newLine;
            // update the size of the preceding line
            double add = this->thisLineHeight + (this->newLineIndex == 0 ? 0 : txt.options.lineSpacing);
            this->cursor.y += this->thisLineHeight + add;
            txt.opcodes[this->newLineIndex].data.newLine.first.setTo(this->cursor.x, add);
            txt.opcodes[this->newLineIndex].data.newLine.second = this->currentAlign;
        }
        if (append) {
            this->thisLineHeight = this->currentFont->lineHeight * this->currentScale;
            txt.opcodes.push_back(TextOpcode(
                NewLine,
                TextOpcodeData(Dimensions(), this->currentAlign)
            ));
            this->cursor.x = this->trailingWhitespace = 0;
            this->newLineIndex = txt.opcodes.size() - 1;
        }
        this->tabIndex = 0;
    }

    void flushWordSegment(BitmapText &txt) {
        if (this->wordSegment.length > 0) {
            TextParser::word.push_back(TextOpcode(TextBlock, TextOpcodeData(this->wordSegment)));
            this->wordSegment.setTo(&this->wordSegment.str[this->wordSegment.length], 0);
            this->wordLength += this->wordSegmentLength;
            this->wordTrailingWhitespace = this->wordSegmentTrailingWhitespace;
            this->wordSegmentLength = 0;
            this->wordSegmentTrailingWhitespace = 0;
        }
    }

    void flushWord(BitmapText &txt) {
        this->flushWordSegment(txt);
        if (!TextParser::word.empty()) {
            this->trailingWhitespace = this->wordTrailingWhitespace;
            double baseScale = static_cast<double>(txt.options.size) / this->currentFont->size;
            if (txt.options.wordWrap && this->cursor.x > 0 && this->cursor.x - this->trailingWhitespace + this->wordLength > txt.dimensions.width() / baseScale) {
                this->newLine(txt, true);
                this->cursor.x = this->wordLength;
            } else {
                this->cursor.x += this->wordLength;
            }
            for (TextOpcode &op: TextParser::word) {
                txt.opcodes.push_back(op);
            }
            TextParser::word.clear();
            this->thisLineHeight = std::max(this->wordHeight, this->thisLineHeight);
            this->wordLength = 0;
            this->wordHeight = 0;
            this->wordTrailingWhitespace = 0;
        }
    }

    void addOp(BitmapText &txt, TextOpcode op) {
        switch (op.type) {
            case SetScale: {
                double v = op.data.number;
                TextParser::stack.scale.push(this->currentScale = v);
                TextParser::word.push_back(op);
                break;
            }
            case PopScale: {
                TextParser::stack.scale.pop();
                this->currentScale = TextParser::stack.scale.top();
                TextParser::word.push_back(TextOpcode(SetScale, TextOpcodeData(this->currentScale)));
                break;
            }
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
                CustomRenderFunction *f = op.data.custom;
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
            case SetFont: {
                BitmapFont *f = op.data.font;
                this->flushWord(txt);
                TextParser::stack.font.push(this->currentFont = f);
                TextParser::word.push_back(op);
                break;
            }
            case PopFont: {
                this->flushWord(txt);
                TextParser::stack.font.pop();
                this->currentFont = TextParser::stack.font.top();
                TextParser::word.push_back(TextOpcode(SetFont, TextOpcodeData(this->currentFont)));
                break;
            }
            case RenderSprite: {
                auto sprite = op.data.sprite;
                auto size = sprite->getSize();
                double imageWidth = size.width() * currentScale;
                TextParser::word.push_back(op);
                this->wordTrailingWhitespace = 0;
                this->wordLength += imageWidth;
                this->wordHeight = std::max(wordHeight, size.height());
                this->thisLineHeight = std::max(this->wordHeight, this->thisLineHeight);
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

GlyphRenderStack TextParser::stack;
std::vector<TextOpcode> TextParser::word;

BitmapText::BitmapText(const BitmapTextOptions &options):
    font(options.font),
    options(options)
{}

BitmapText &BitmapText::setFont(std::shared_ptr<BitmapFont> font) {
    this->font = font;
    return *this;
}

BitmapText &BitmapText::setText(const std::string &text) {
    if (this->rich || text != this->text) {
        this->text = text;
        this->rich = false;
        this->dirty = true;
    }
    return *this;
}

BitmapText &BitmapText::setRichText(const std::string &text) {
    if (!this->rich || text != this->text) {
        this->text = text;
        this->rich = true;
        this->dirty = true;
    }
    return *this;
}

BitmapText &BitmapText::refresh() {
    if (this->dirty) {
        TextParser parser;
        parser.parseText(*this, this->text, this->rich);
        this->dirty = false;
    }
    return *this;
}

void BitmapText::resize(double w, double h) {
    if (this->options.wordWrap) {
        if ((this->dimensions.width() != static_cast<int>(w)) || (this->dimensions.height() != static_cast<int>(h))) {
            this->dirty = true;
            this->dimensions.setTo(w, h);
        }
    } else {
        this->dimensions.setTo(w / this->scale.x, h / this->scale.y);
    }
}

void BitmapText::render(RenderContext &ctx) {
    this->refresh();
    Color color = this->color;
    double baseScale = static_cast<double>(this->options.size) / this->font->size;
    ScaleFactor scale;
    double thisLineHeight = 0;
    int lastId = -1;
    Point cursor;
    CustomRenderFunction *custom = nullptr;
    double totalWidth = this->options.wordWrap ? this->dimensions.width() : this->textDimensions.width();
    int charCount = this->charCount;
    size_t tabIndex = 0;
    BitmapFont *font = this->font.get();
    for (TextOpcode &op: this->opcodes) {
        switch (op.type) {
            case SetColor: {
                color = this->color * op.data.color;
                break;
            }
            case SetScale: {
                scale.setTo(op.data.number, op.data.number);
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
                    (totalWidth - dims.width()) * this->scale.x * baseScale * align,
                    cursor.y + thisLineHeight * this->scale.y * baseScale
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
                if (this->options.dynamic && custom) {
                    custom(&ctx, this, &renderData);
                }
                auto size = sprite->getSize();
                sprite->scale.setTo(baseScale * scale.x, baseScale * scale.y);
                sprite->position.setTo(this->position.x + renderData.position.x, this->position.y + renderData.position.y + (thisLineHeight * scale.y * baseScale - size.height()));
                Color originalColor(sprite->color);
                sprite->color = sprite->color * this->color;
                sprite->render(ctx);
                sprite->color = originalColor;
                cursor.x += size.width();
                break;
            }
            case SetFont: {
                font = op.data.font;
                break;
            }
            case TextBlock: {
                StringSlice &txt = op.data.text;
                for (size_t i = 0; i < txt.length; ++i) {
                    if (charCount > -1 && --charCount < 0) {
                        return;
                    }
                    unsigned char c = txt[i];
                    GlyphData glyph = font->getGlyph(c);
                    if (glyph.id && glyph.id == c) {
                        int xAdvance = glyph.xAdvance;
                        GlyphRenderData renderData(
                            c,
                            color,
                            scale,
                            cursor
                        );
                        if (lastId != -1) {
                            double kern = font->kern(lastId, glyph.id) * renderData.scale.x * this->scale.x * baseScale;
                            if (kern < 0) {
                                cursor.x += kern;
                                renderData.position.x += kern;
                            }
                        }
                        if (c == ' ' || c == '\t') {
                            // it's a space; just advance the cursor and move on
                            lastId = -1;
                        // TODO: escape characters
                        } else {
                            if (this->options.dynamic && custom) {
                                custom(&ctx, this, &renderData);
                            }
                            Matrix matrix;
                            matrix
                                .translate(glyph.offset.x, glyph.offset.y)
                                .scale(renderData.scale.x * this->scale.x * baseScale, renderData.scale.y * this->scale.y * baseScale)
                                .translate(this->position.x + renderData.position.x, this->position.y + renderData.position.y);
                            DrawKey key;
                            key.image = font->getPage(glyph.page);
                            key.smooth = this->smooth;
                            key.blend = this->blendMode;
                            ctx.addRect(key, glyph.rect, matrix, renderData.color);
                        }
                        cursor.x += xAdvance * renderData.scale.x * this->scale.x * baseScale;
                        lastId = glyph.id;
                    } else {
                        lastId = -1;
                    }
                }
                break;
            }
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
