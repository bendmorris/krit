#include "krit/sprites/Text.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <math.h>
#include <memory>
#include <stack>
#include <stdio.h>
#include <string>
#include <string_view>
#include <utility>

#include "harfbuzz/hb.h"
#include "krit/App.h"
#include "krit/Camera.h"
#include "krit/math/Matrix.h"
#include "krit/math/Point.h"
#include "krit/math/ScaleFactor.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageRegion.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Renderer.h"
#include "krit/render/SmoothingMode.h"
#include "krit/utils/Utf8.h"

namespace krit {

static const int FONT_SCALE = 64;

enum TextOpcodeType : int {
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
    Whitespace,
    CharDelay,
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
    while (!stack.empty())
        stack.pop();
}

struct TextRenderStack {
    std::stack<Color> color;
    std::stack<AlignType> align;
    std::stack<CustomTextRenderFunction *> custom;

    TextRenderStack() {}

    void clear() {
        clearStack(this->color);
        clearStack(this->align);
        clearStack(this->custom);
    }
};

void TextOpcode::debugPrint() {
    switch (this->type) {
        case SetColor: {
            printf("SetColor");
            break;
        }
        case SetAlign: {
            printf("SetAlign");
            break;
        }
        case SetCustom: {
            printf("SetCustom");
            break;
        }
        case PopColor: {
            printf("PopColor");
            break;
        }
        case PopAlign: {
            printf("PopAlign");
            break;
        }
        case PopCustom: {
            printf("PopCustom");
            break;
        }
        case GlyphBlock: {
            printf("GlyphBlock");
            break;
        }
        case NewLine: {
            printf("NewLine");
            break;
        }
        case RenderSprite: {
            printf("RenderSprite");
            break;
        }
        case Tab: {
            printf("Tab");
            break;
        }
        case Whitespace: {
            printf("Whitespace");
            break;
        }
        case CharDelay: {
            printf("CharDelay");
            break;
        }
    }
}

/**
 * Utility structure to parse format tags from a string of text and generate a
 * Vector of TextOpcodes.
 */
struct TextParser {
    static TextRenderStack stack;

    Point cursor;

    AlignType currentAlign = LeftAlign;
    int newLineIndex = 0;
    size_t tabIndex = 0;

    std::vector<TextOpcode> word;
    int wordLength = 0;

    hb_buffer_t *hbBuf;

    void parseText(Text &txt, const std::string &s, bool rich) {
        txt.opcodes.clear();
        txt.textDimensions.setTo(0, 0);
        txt.maxChars = 0;

        if (s.empty()) {
            return;
        }

        if (txt.hbBuf) {
            hb_buffer_clear_contents(txt.hbBuf);
        } else {
            txt.hbBuf = hb_buffer_create();
        }
        hb_buffer_set_direction(txt.hbBuf, HB_DIRECTION_LTR);
        hb_buffer_set_script(txt.hbBuf, HB_SCRIPT_LATIN);
        hb_buffer_set_language(txt.hbBuf, hb_language_from_string("en", -1));
        hb_buffer_set_cluster_level(
            txt.hbBuf, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);

        hb_buffer_t *hbBuf = txt.hbBuf;

        // find all format tags, and generate a string with the text minus tags
        // vector of (pos, length) tag locations from < to >
        std::vector<std::pair<size_t, size_t>> tagLocations;
        for (Utf8Iterator it = s.begin(); it != s.end(); ++it) {
            size_t i = it.index();
            char32_t c = *it;
            if (c == '<') {
                for (; it != s.end(); ++it) {
                    size_t i2 = it.index();
                    char32_t c2 = *it;
                    if (c2 == '>') {
                        tagLocations.emplace_back(i, i2 - i + 1);
                        break;
                    }
                }
            } else if (c == '\n') {
                tagLocations.emplace_back(i, 1);
            }
        }
        std::string rawText;
        std::vector<std::pair<size_t, std::string_view>> tags;
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
                if (s[offset + len - 1] == '>') {
                    --len;
                }
                if (s[offset + len - 1] == '/') {
                    --len;
                }
                tags.emplace_back(l.first - tagNameOffset,
                                  std::string_view(&s[offset], len));
                tagNameOffset += l.second;
                if (l.first + l.second < s.size()) {
                    size_t startPos = l.first + l.second;
                    if (i < tagLocations.size() - 1) {
                        // add text from here to next tag start
                        rawText.append(&s[startPos],
                                       tagLocations[i + 1].first - startPos);
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

        txt.font->shape(hbBuf, txt.size * FONT_SCALE);
        unsigned int glyphCount;
        hb_glyph_info_t *glyphInfo =
            hb_buffer_get_glyph_infos(hbBuf, &glyphCount);
        hb_glyph_position_t *glyphPos =
            hb_buffer_get_glyph_positions(hbBuf, &glyphCount);

        // iterate over characters/tags and handle them
        TextRenderStack &st = TextParser::stack;
        st.clear();
        st.color.push(txt.baseColor);
        st.align.push(this->currentAlign = txt.align);
        st.custom.push(nullptr);
        word.clear();
        txt.opcodes.push_back(
            TextOpcode(NewLine, TextOpcodeData(Dimensions(), txt.align)));
        txt.maxChars = rawText.size();

        hb_font_extents_t extents;
        hb_font_get_h_extents(txt.font->font, &extents);
        txt.lineHeight = (extents.ascender - extents.descender +
                          extents.line_gap); // * txt.size / FONT_SCALE;
        newLineIndex = 0;

        size_t tagPointer = 0;
        Utf8Iterator it = rawText.begin();
        for (size_t i = 0; i < glyphCount; ++i) {
            hb_glyph_info_t &_glyphInfo = glyphInfo[i];
            hb_glyph_position_t &_glyphPos = glyphPos[i];
            size_t txtPointer = _glyphInfo.cluster;
            // handle any tags that came before this glyph
            while (tagPointer < tags.size() &&
                   tags[tagPointer].first <= txtPointer) {
                this->addTag(txt, tags[tagPointer++].second);
            }
            // handle this glyph
            while (it.index() < txtPointer) {
                ++it;
            }
            char32_t c = *it;
            switch (c) {
                case ' ': {
                    flushWord(txt);
                    cursor.x += _glyphPos.x_advance;
                    if (txt.opcodes.back().type == Whitespace) {
                        txt.opcodes.back().data.number += _glyphPos.x_advance;
                    } else {
                        txt.opcodes.push_back(TextOpcode(
                            Whitespace,
                            TextOpcodeData((float)_glyphPos.x_advance)));
                    }
                    break;
                }
                default: {
                    auto &back = word.back();
                    if (!word.empty() && back.type == GlyphBlock &&
                        back.data.glyphBlock.startIndex +
                                back.data.glyphBlock.glyphs ==
                            i) {
                        // add this glyph to the existing opcode
                        ++back.data.glyphBlock.glyphs;
                        wordLength += _glyphPos.x_advance;
                        // cursor.x += _glyphPos.x_advance;
                    } else {
                        word.emplace_back(
                            TextOpcode(GlyphBlock, TextOpcodeData(i, 1, 0)));
                        wordLength += _glyphPos.x_advance;
                    }
                }
            }
        }
        while (tagPointer < tags.size()) {
            this->addTag(txt, tags[tagPointer++].second);
        }

        this->newLine(txt);

        // for (auto &op : txt.opcodes) {
        //     op.debugPrint();
        //     printf(" ");
        // }
        // puts("");

        txt.maxChars = glyphCount;
        for (auto it : txt.opcodes) {
            if (it.type == CharDelay) {
                txt.maxChars += it.data.charDelay;
            }
        }
    }

    void flushWord(Text &txt) {
        if (txt.wordWrap && (cursor.x + wordLength) * txt.size / FONT_SCALE >
                                txt.dimensions.width()) {
            newLine(txt, false);
        }
        txt.opcodes.insert(txt.opcodes.end(),
                           std::make_move_iterator(word.begin()),
                           std::make_move_iterator(word.end()));
        word.clear();
        cursor.x += wordLength;
        wordLength = 0;
    }

    void addTag(Text &txt, std::string_view tagName) {
        static std::string tagStr;
        bool close = false;
        if (tagName[0] == '/') {
            close = true;
            tagName = std::string_view(&tagName[1], tagName.length() - 1);
        }
        tagStr.assign(tagName.data(), tagName.length());
        auto found = Text::formatTags.find(tagStr);
        if (found != Text::formatTags.end()) {
            TextFormatTagOptions &tag = found->second;
            if (tag.color.present) {
                if (close)
                    this->addOp(txt, TextOpcode(PopColor));
                else
                    this->addOp(
                        txt,
                        TextOpcode(SetColor, TextOpcodeData(tag.color.value)));
            }
            if (tag.align.present) {
                if (close)
                    this->addOp(txt, TextOpcode(PopAlign));
                else
                    this->addOp(
                        txt,
                        TextOpcode(SetAlign, TextOpcodeData(tag.align.value)));
            }
            if (tag.custom != nullptr) {
                if (close)
                    this->addOp(txt, TextOpcode(PopCustom));
                else
                    this->addOp(
                        txt, TextOpcode(SetCustom, TextOpcodeData(tag.custom)));
            }
            if (tag.sprite) {
                this->addOp(
                    txt, TextOpcode(RenderSprite, TextOpcodeData(tag.sprite)));
            }
            if (tag.newline && !close) {
                this->addOp(
                    txt, TextOpcode(NewLine,
                                    TextOpcodeData(Dimensions(), LeftAlign)));
            }
            if (tag.tab && !close) {
                this->addOp(txt, TextOpcode(Tab, TextOpcodeData()));
            }
            if (tag.charDelay && !close) {
                this->addOp(
                    txt, TextOpcode(CharDelay, TextOpcodeData(tag.charDelay)));
            }
        }
    }

    void newLine(Text &txt, bool canBreak = true) {
        if (canBreak) {
            this->flushWord(txt);
        }
        if (txt.opcodes[this->newLineIndex].type == NewLine) {
            // std::pair<Dimensions, AlignType> &align =
            // txt.opcodes[this->newLineIndex].data.newLine; update the size of
            // the preceding line
            float add = this->newLineIndex == 0 ? 0 : txt.lineSpacing;
            this->cursor.y += txt.lineHeight + add;
            txt.opcodes[this->newLineIndex].data.newLine.first.setTo(
                this->cursor.x, txt.lineHeight + add);
            txt.opcodes[this->newLineIndex].data.newLine.second =
                this->currentAlign;
        }
        txt.opcodes.push_back(TextOpcode(
            NewLine, TextOpcodeData(Dimensions(), this->currentAlign)));
        this->newLineIndex = txt.opcodes.size() - 1;
        this->tabIndex = 0;
        txt.textDimensions.setTo(std::max(txt.textDimensions.width(),
                                          cursor.x * txt.size / FONT_SCALE),
                                 cursor.y * txt.size / FONT_SCALE);
        cursor.x = 0;
    }

    void addOp(Text &txt, TextOpcode op) {
        switch (op.type) {
            case SetColor: {
                Color &v = op.data.color;
                TextParser::stack.color.push(v);
                word.push_back(op);
                break;
            }
            case PopColor: {
                TextParser::stack.color.pop();
                word.push_back(TextOpcode(
                    SetColor, TextOpcodeData(TextParser::stack.color.top())));
                break;
            }
            case SetCustom: {
                CustomTextRenderFunction *f = op.data.custom;
                TextParser::stack.custom.push(f);
                word.push_back(op);
                break;
            }
            case PopCustom: {
                TextParser::stack.custom.pop();
                word.push_back(TextOpcode(
                    SetCustom, TextOpcodeData(TextParser::stack.custom.top())));
                break;
            }
            case NewLine: {
                newLine(txt);
                break;
            }
            case SetAlign: {
                AlignType &v = op.data.align;
                if (this->cursor.x + wordLength > 0) {
                    this->newLine(txt);
                }
                TextParser::stack.align.push(this->currentAlign = v);
                break;
            }
            case PopAlign: {
                TextParser::stack.align.pop();
                if (this->cursor.x + wordLength > 0) {
                    this->newLine(txt);
                }
                this->currentAlign = TextParser::stack.align.top();
                break;
            }
            case RenderSprite: {
                auto sprite = op.data.sprite;
                auto size = sprite->getSize();
                float imageWidth = size.width();
                word.push_back(op);
                wordLength += imageWidth;
                break;
            }
            case Tab: {
                if (this->tabIndex < txt.tabStops.size()) {
                    flushWord(txt);
                    cursor.x = txt.tabStops[this->tabIndex];
                    word.push_back(op);
                }
                break;
            }
            default: {
                word.push_back(op);
            }
        }
    }
};

TextRenderStack TextParser::stack;

Text::Text(const TextOptions &options) : TextOptions(options) { assert(font); }

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

void Text::resize(float w, float h) {
    if (this->wordWrap) {
        if ((this->dimensions.width() != static_cast<int>(w)) ||
            (this->dimensions.height() != static_cast<int>(h))) {
            this->dirty = true;
            this->dimensions.setTo(w, h);
        }
    } else {
        this->dimensions.setTo(w, h);
    }
}

void Text::render(RenderContext &ctx) {
    this->refresh();

    Color color = this->color * this->baseColor;
    Point cursor;
    CustomTextRenderFunction *custom = nullptr;
    float totalWidth = this->wordWrap ? this->dimensions.width()
                                      : this->textDimensions.width();
    int charCount = this->charCount;
    size_t tabIndex = 0;

    float cameraScale = std::max(ctx.camera->scale.x, ctx.camera->scale.y);
    float size = this->size * cameraScale;
    float fontScale = std::floor(size) / cameraScale / 64.0;
    bool pixelPerfect = allowPixelPerfect && fontScale < 20;

    for (TextOpcode &op : this->opcodes) {
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
                float align;
                switch (op.data.newLine.second) {
                    case LeftAlign:
                        align = 0;
                        break;
                    case CenterAlign:
                        align = 0.5;
                        break;
                    case RightAlign:
                        align = 1;
                        break;
                }
                cursor.setTo((totalWidth - dims.width()) * align,
                             cursor.y + lineHeight);
                break;
            }
            case RenderSprite: {
                auto sprite = op.data.sprite;
                GlyphRenderData renderData(cursor);
                if (custom) {
                    custom(&ctx, this, &renderData);
                }
                auto size = sprite->getSize();
                sprite->scale.setTo(scale);
                sprite->position.setTo(
                    this->position.x + renderData.position.x,
                    this->position.y + renderData.position.y +
                        (lineHeight * scale.y - size.height()));
                Color originalColor(sprite->color);
                sprite->color = sprite->color * this->color;
                sprite->render(ctx);
                sprite->color = originalColor;
                cursor.x += size.width();
                break;
            }
            case CharDelay: {
                if (charCount > -1) {
                    charCount -= op.data.charDelay;
                    if (charCount <= 0) {
                        return;
                    }
                }
                break;
            }
            case GlyphBlock: {
                unsigned int glyphCount;
                hb_glyph_info_t *glyphInfo =
                    hb_buffer_get_glyph_infos(hbBuf, &glyphCount);
                hb_glyph_position_t *glyphPos =
                    hb_buffer_get_glyph_positions(hbBuf, &glyphCount);
                for (size_t i = op.data.glyphBlock.startIndex;
                     i <
                     op.data.glyphBlock.startIndex + op.data.glyphBlock.glyphs;
                     ++i) {
                    hb_glyph_info_t _info = glyphInfo[i];
                    hb_glyph_position_t _pos = glyphPos[i];
                    GlyphData &glyph =
                        font->getGlyph(_info.codepoint, std::floor(size));

                    GlyphRenderData renderData(
                        _info.codepoint, color,
                        scale, // FIXME
                        Point((cursor.x + _pos.x_offset) * fontScale,
                              (cursor.y - _pos.y_offset) * fontScale));
                    if (custom) {
                        custom(&ctx, this, &renderData);
                    }
                    Matrix matrix;
                    DrawKey key;
                    if (pixelPerfect) {
                        key.smooth = SmoothingMode::SmoothNearest;
                        matrix.translate(position.x, position.y);
                        ctx.camera->transformMatrix(matrix);
                        matrix.tx = std::round(matrix.tx);
                        matrix.ty = std::round(matrix.ty);
                        matrix.a = matrix.d = 1;
                        matrix.b = matrix.c = 0;
                        matrix.translate(
                            std::round(renderData.position.x * cameraScale),
                            std::round(renderData.position.y * cameraScale));
                        matrix.translate(std::round(glyph.offset.x),
                                         std::round(-glyph.offset.y));
                    } else {
                        key.smooth = smooth == SmoothingMode::SmoothMipmap
                                         ? SmoothingMode::SmoothLinear
                                         : this->smooth;
                        matrix.translate(position.x, position.y);
                        matrix.translate(renderData.position.x,
                                         renderData.position.y);
                        ctx.camera->transformMatrix(matrix);
                        matrix.a = matrix.d = 1;
                        matrix.b = matrix.c = 0;
                        matrix.translate(glyph.offset.x, -glyph.offset.y);
                    }
                    key.image = glyph.region.img;
                    key.blend = blendMode;
                    key.shader = this->shader
                                     ? this->shader
                                     : ctx.app->renderer.getDefaultTextShader();
                    ctx.addRectRaw(key, glyph.region.rect, matrix,
                                   renderData.color);

                    cursor.x += _pos.x_advance;
                    if (charCount > -1 && --charCount <= 0) {
                        return;
                    }
                }
                break;
            }
            case Whitespace: {
                cursor.x += op.data.number;
                break;
            }
            case Tab: {
                cursor.x = this->tabStops[tabIndex++];
                break;
            }
            // other opcodes (e.g. the Pop variants) are removed during parsing
            default: {
            }
        }
    }
}

Text &Text::setText(const std::string &text) {
    if (this->rich || text != this->text) {
        this->text = text;
        this->rich = false;
        this->dirty = true;
    }
    return *this;
}

Text &Text::setRichText(const std::string &text) {
    if (!this->rich || text != this->text) {
        this->text = text;
        this->rich = true;
        this->dirty = true;
    }
    return *this;
}

}
