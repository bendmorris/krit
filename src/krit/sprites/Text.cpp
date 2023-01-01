#include "krit/sprites/Text.h"
#include "harfbuzz/hb.h"
#include "krit/App.h"
#include "krit/Camera.h"
#include "krit/math/Matrix.h"
#include "krit/math/Point.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageRegion.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Renderer.h"
#include "krit/render/SmoothingMode.h"
#include "krit/utils/Utf8.h"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <math.h>
#include <memory>
#include <sstream>
#include <stack>
#include <stdio.h>
#include <string>
#include <string_view>
#include <utility>
#if TRACY_ENABLE
#include "krit/tracy/Tracy.hpp"
#endif

namespace krit {

static const int FONT_SCALE = 64;

std::unordered_map<std::string, TextFormatTagOptions> Text::formatTags = {
    {"\n", TextFormatTagOptions().setNewline()},
    {"br", TextFormatTagOptions().setNewline()},
    {"left", TextFormatTagOptions().setAlign(LeftAlign)},
    {"center", TextFormatTagOptions().setAlign(CenterAlign)},
    {"right", TextFormatTagOptions().setAlign(RightAlign)},
    {"\t", TextFormatTagOptions().setTab()},
    {"tab", TextFormatTagOptions().setTab()},
    {"b", TextFormatTagOptions().setBorder()},
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
    std::stack<CustomTextRenderFunction> custom;

    TextRenderStack() {}

    void clear() {
        clearStack(this->color);
        clearStack(this->align);
        clearStack(this->custom);
    }
};

static std::unordered_map<hb_codepoint_t, int> charDelays = {
    {'.', 10}, {',', 6}, {'!', 10}, {'?', 10}, {'-', 6}, {';', 4}, {':', 4},
};

// void TextOpcode::debugPrint() {
//     switch (this->type) {
//         case SetColor: {
//             printf("SetColor");
//             break;
//         }
//         case SetAlign: {
//             printf("SetAlign");
//             break;
//         }
//         case SetCustom: {
//             printf("SetCustom");
//             break;
//         }
//         case PopColor: {
//             printf("PopColor");
//             break;
//         }
//         case PopAlign: {
//             printf("PopAlign");
//             break;
//         }
//         case PopCustom: {
//             printf("PopCustom");
//             break;
//         }
//         case GlyphBlock: {
//             printf("GlyphBlock");
//             break;
//         }
//         case NewLine: {
//             printf("NewLine");
//             break;
//         }
//         case RenderSprite: {
//             printf("RenderSprite");
//             break;
//         }
//         case Tab: {
//             printf("Tab");
//             break;
//         }
//         case Whitespace: {
//             printf("Whitespace");
//             break;
//         }
//         case CharDelay: {
//             printf("CharDelay");
//             break;
//         }
//         case EnableBorder: {
//             printf("EnableBorder");
//             break;
//         }
//         case DisableBorder: {
//             printf("DisableBorder");
//             break;
//         }
//     }
// }

struct FormatTag {
    std::string_view name;
    std::vector<std::pair<std::string_view, std::string_view>> attributes;
};

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
#if TRACY_ENABLE
        ZoneScopedN("TextParser::parseText");
#endif
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
            } else if (c == '\n' || c == '\t') {
                tagLocations.emplace_back(i, 1);
            }
        }
        auto &rawText = txt.rawText;
        rawText.clear();
        std::vector<std::pair<size_t, FormatTag>> tags;
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
                // add the tag and find any attributes inside;
                // this parses in one pass and does not handle malformed tags!
                FormatTag tag;
                size_t start = offset;
                if (s[offset] == '\n') {
                    tag.name = std::string_view(&s[offset], 1);
                } else {
                    for (size_t o = offset; o < offset + len; ++o) {
                        if (s[o] == ' ') {
                            if (tag.name.empty()) {
                                // we have the tag name
                                tag.name = std::string_view(&s[offset], o - offset);
                            }
                            start = o + 1;
                        } else if (s[o] == '=') {
                            // we have the attribute name; parse the attribute value
                            tag.attributes.emplace_back();
                            tag.attributes.back().first =
                                std::string_view(&s[start], o - start);
                            assert(s[o + 1] == '"' || s[o + 1] == '\'');
                            for (size_t o2 = o + 2; o2 < offset + len; ++o2) {
                                if (s[o2] == s[o + 1]) {
                                    tag.attributes.back().second =
                                        std::string_view(&s[o + 2], o2 - (o + 2));
                                    o = o2;
                                }
                            }
                        }
                    }
                    if (tag.name.empty()) {
                        // there were no attributes, so the whole thing is the tag
                        // name
                        tag.name = std::string_view(&s[offset], len);
                    }
                }
                // printf("tag: name %.*s (%zu)\n", (int)tag.name.size(),
                // tag.name.data(), tag.name.size()); for (size_t _i = 0; _i <
                // tag.attributes.size(); ++_i) {
                //     printf("  attribute %.*s=%.*s\n",
                //            (int)tag.attributes[_i].first.size(),
                //            tag.attributes[_i].first.data(),
                //            (int)tag.attributes[_i].second.size(),
                //            tag.attributes[_i].second.data());
                // }
                // std::string_view(&s[offset], len)
                tags.emplace_back(l.first - tagNameOffset, tag);
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
        txt.opcodes.emplace_back(std::in_place_index_t<NewLine>(), Dimensions(),
                                 txt.align);
        txt.hasBorderTags = false;

        hb_font_extents_t extents;
        hb_font_get_h_extents(txt.font->font, &extents);
        txt.lineHeight = (extents.ascender - extents.descender +
                          extents.line_gap); // * txt.size / FONT_SCALE;
        newLineIndex = 0;

        size_t tagPointer = 0;
        Utf8Iterator it = rawText.begin();
        int glyphCost = 0;
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
                    cursor.x() += _glyphPos.x_advance;
                    if (txt.opcodes.back().index() == Whitespace) {
                        std::get<Whitespace>(txt.opcodes.back()) +=
                            _glyphPos.x_advance;
                    } else {
                        txt.opcodes.emplace_back(
                            std::in_place_index_t<Whitespace>(),
                            _glyphPos.x_advance);
                    }
                    break;
                }
                default: {
                    glyphCost += std::max(charDelays[c], 1);
                    if (!word.empty() && word.back().index() == GlyphBlock &&
                        std::get<GlyphBlock>(word.back()).startIndex +
                                std::get<GlyphBlock>(word.back()).glyphs ==
                            i) {
                        auto &back = word.back();
                        // add this glyph to the existing opcode
                        ++std::get<GlyphBlock>(back).glyphs;
                        wordLength += _glyphPos.x_advance;
                        // cursor.x += _glyphPos.x_advance;
                    } else {
                        word.emplace_back(std::in_place_index_t<GlyphBlock>(),
                                          i, 1, 0);
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

        txt.maxChars = glyphCost;
        for (auto it : txt.opcodes) {
            if (it.index() == CharDelay) {
                txt.maxChars += std::get<CharDelay>(it);
            } else if (it.index() == Whitespace) {
                ++txt.maxChars;
            }
        }

        if (txt.wordWrap) {
            txt.renderedSize.copyFrom(txt.dimensions);
            txt.dimensions.y() = txt.textDimensions.y();
        } else {
            txt.dimensions.copyFrom(txt.textDimensions);
        }
    }

    void flushWord(Text &txt) {
        if (txt.wordWrap && (cursor.x() + wordLength) * txt.size / FONT_SCALE >
                                txt.dimensions.x()) {
            newLine(txt, false);
        }
        txt.opcodes.insert(txt.opcodes.end(),
                           std::make_move_iterator(word.begin()),
                           std::make_move_iterator(word.end()));
        word.clear();
        cursor.x() += wordLength;
        wordLength = 0;
    }

    void addTag(Text &txt, const FormatTag &tag) {
        // printf("add tag: %.*s\n", (int)tag.name.size(), tag.name.data());
        std::string_view tagName = tag.name;
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
            if (tag.color) {
                if (close)
                    this->addOp(txt,
                                TextOpcode(std::in_place_index_t<PopColor>()));
                else
                    this->addOp(txt,
                                TextOpcode(std::in_place_index_t<SetColor>(),
                                           tag.color.value()));
            }
            if (tag.align) {
                if (close)
                    this->addOp(txt,
                                TextOpcode(std::in_place_index_t<PopAlign>()));
                else
                    this->addOp(txt,
                                TextOpcode(std::in_place_index_t<SetAlign>(),
                                           tag.align.value()));
            }
            if (tag.custom != nullptr) {
                if (close)
                    this->addOp(txt,
                                TextOpcode(std::in_place_index_t<PopCustom>()));
                else
                    this->addOp(txt,
                                TextOpcode(std::in_place_index_t<SetCustom>(),
                                           tag.custom));
            }
            if (tag.sprite) {
                this->addOp(txt,
                            TextOpcode(std::in_place_index_t<RenderSprite>(),
                                       (tag.sprite)));
            }
            if (tag.newline && !close) {
                this->addOp(txt, TextOpcode(std::in_place_index_t<NewLine>(),
                                            Dimensions(), LeftAlign));
            }
            if (tag.tab && !close) {
                this->addOp(txt, TextOpcode(std::in_place_index_t<Tab>()));
            }
            if (tag.charDelay && !close) {
                this->addOp(txt, TextOpcode(std::in_place_index_t<CharDelay>(),
                                            (tag.charDelay)));
            }
            if (tag.border) {
                if (close) {
                    this->addOp(
                        txt,
                        TextOpcode(std::in_place_index_t<DisableBorder>()));
                } else {
                    this->addOp(
                        txt, TextOpcode(std::in_place_index_t<EnableBorder>()));
                }
            }
        }
    }

    void newLine(Text &txt, bool canBreak = true) {
        if (canBreak) {
            this->flushWord(txt);
        }
        float trailingWhitespace = 0;
        if (txt.opcodes.back().index() == Whitespace) {
            trailingWhitespace = std::get<Whitespace>(txt.opcodes.back());
        }
        if (txt.opcodes[this->newLineIndex].index() == NewLine) {
            // std::pair<Dimensions, AlignType> &align =
            // txt.opcodes[this->newLineIndex].data.newLine; update the size of
            // the preceding line
            float add = this->newLineIndex == 0 ? 0 : txt.lineSpacing;
            this->cursor.y() += txt.lineHeight + add;
            std::get<NewLine>(txt.opcodes[this->newLineIndex])
                .first.setTo(this->cursor.x() - trailingWhitespace,
                             txt.lineHeight + add);
            std::get<NewLine>(txt.opcodes[this->newLineIndex]).second =
                this->currentAlign;
        }
        txt.opcodes.emplace_back(std::in_place_index_t<NewLine>(), Dimensions(),
                                 this->currentAlign);
        this->newLineIndex = txt.opcodes.size() - 1;
        this->tabIndex = 0;
        txt.textDimensions.setTo(
            std::max(txt.textDimensions.x(),
                     (cursor.x() - trailingWhitespace) * txt.size / FONT_SCALE),
            cursor.y() * txt.size / FONT_SCALE);
        cursor.x() = 0;
    }

    void addOp(Text &txt, TextOpcode op) {
        switch (op.index()) {
            case SetColor: {
                Color &v = std::get<SetColor>(op);
                TextParser::stack.color.push(v);
                word.push_back(op);
                break;
            }
            case PopColor: {
                TextParser::stack.color.pop();
                word.emplace_back(std::in_place_index_t<SetColor>(),
                                  TextParser::stack.color.top());
                break;
            }
            case SetCustom: {
                CustomTextRenderFunction f = std::get<SetCustom>(op);
                TextParser::stack.custom.push(f);
                word.push_back(op);
                break;
            }
            case PopCustom: {
                TextParser::stack.custom.pop();
                word.emplace_back(std::in_place_index_t<SetCustom>(),
                                  TextParser::stack.custom.top());
                break;
            }
            case NewLine: {
                newLine(txt);
                break;
            }
            case SetAlign: {
                AlignType &v = std::get<SetAlign>(op);
                if (this->cursor.x() + wordLength > 0) {
                    this->newLine(txt);
                }
                TextParser::stack.align.push(this->currentAlign = v);
                break;
            }
            case PopAlign: {
                TextParser::stack.align.pop();
                if (this->cursor.x() + wordLength > 0) {
                    this->newLine(txt);
                }
                this->currentAlign = TextParser::stack.align.top();
                break;
            }
            case RenderSprite: {
                auto sprite = std::get<RenderSprite>(op);
                auto size = sprite->getSize();
                float imageWidth = size.x();
                word.push_back(op);
                wordLength += imageWidth;
                break;
            }
            case Tab: {
                if (this->tabIndex < txt.tabStops.size()) {
                    flushWord(txt);
                    cursor.x() = txt.tabStops[this->tabIndex];
                    word.push_back(op);
                }
                break;
            }
            case EnableBorder: {
                txt.hasBorderTags = true;
            }
            default: {
                word.push_back(op);
            }
        }
    }
};

TextRenderStack TextParser::stack;

TextOptions &TextOptions::setFont(const std::string &name) {
    this->font = App::ctx.engine->fonts.getFont(name);
    assert(this->font);
    return *this;
}

Text::Text(const TextOptions &options) : TextOptions(options) { assert(font); }

Text::~Text() {
    if (hbBuf) {
        hb_buffer_destroy(hbBuf);
    }
}

Text &Text::refresh() {
    if (this->dirty || (this->wordWrap && dimensions.x() != renderedSize.x())) {
        // TODO: reuse static parser...
        TextParser parser;
        parser.parseText(*this, this->text, this->rich);
        this->dirty = false;
    }
    return *this;
}

void Text::resize(float w, float h) {
    if (this->wordWrap) {
        if ((this->dimensions.x() != static_cast<int>(w)) ||
            (this->dimensions.y() != static_cast<int>(h))) {
            this->dirty = true;
            this->dimensions.setTo(w, h);
        }
    } else {
        this->dimensions.setTo(w, h);
    }
}

void Text::render(RenderContext &ctx) {
    if (borderThickness > 0 && borderColor.a > 0 &&
        (this->border || hasBorderTags)) {
        __render(ctx, true);
    }
    __render(ctx, false);
}

void Text::__render(RenderContext &ctx, bool border) {
    if (charCount == 0) {
        return;
    }

    this->refresh();

    bool _borderEnabled = this->border;

    Color color = this->color * this->baseColor;
    Point cursor;
    CustomTextRenderFunction custom = nullptr;
    float totalWidth =
        this->wordWrap ? this->dimensions.x() : this->textDimensions.x();
    int charCount = this->charCount;
    size_t tabIndex = 0;

    float cameraScale =
        dynamicSize ? std::max(ctx.camera->scale.x(), ctx.camera->scale.y())
                    : 1;
    float size = this->size * cameraScale;
    float fontScale = std::round(size) / cameraScale / 64.0;
    bool pixelPerfect = allowPixelPerfect && fontScale < 20;

    Utf8Iterator it = rawText.begin();

    for (TextOpcode &op : this->opcodes) {
        switch (op.index()) {
            case SetColor: {
                color = this->color * std::get<SetColor>(op);
                break;
            }
            case SetCustom: {
                custom = std::get<SetCustom>(op);
                break;
            }
            case NewLine: {
                tabIndex = 0;
                Dimensions &dims = std::get<NewLine>(op).first;
                float align;
                switch (std::get<NewLine>(op).second) {
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
                cursor.setTo((totalWidth / fontScale - dims.x()) * align,
                             cursor.y() + lineHeight);
                break;
            }
            case RenderSprite: {
                auto sprite = std::get<RenderSprite>(op);
                auto size = sprite->getSize();
                if (!border) {
                    GlyphRenderData renderData(cursor);
                    if (custom) {
                        custom(&ctx, this, &renderData);
                    }
                    sprite->scale.copyFrom(scale);
                    sprite->position.setTo(
                        this->position.x() + renderData.position.x() * fontScale,
                        this->position.y() + renderData.position.y() * fontScale +
                        (lineHeight * scale.y() * fontScale - size.y()) - lineHeight * fontScale);
                    Color originalColor(sprite->color);
                    sprite->color = sprite->color * color;
                    sprite->render(ctx);
                    sprite->color = originalColor;
                }
                cursor.x() += size.x();
                break;
            }
            case CharDelay: {
                if (charCount > -1) {
                    charCount -= std::get<CharDelay>(op);
                    if (charCount <= 0) {
                        return;
                    }
                }
                break;
            }
            case EnableBorder: {
                _borderEnabled = true;
                break;
            }
            case DisableBorder: {
                _borderEnabled = this->border;
                break;
            }
            case GlyphBlock: {
                unsigned int glyphCount;
                hb_glyph_info_t *glyphInfo =
                    hb_buffer_get_glyph_infos(hbBuf, &glyphCount);
                hb_glyph_position_t *glyphPos =
                    hb_buffer_get_glyph_positions(hbBuf, &glyphCount);
                for (size_t i = std::get<GlyphBlock>(op).startIndex;
                     i < std::get<GlyphBlock>(op).startIndex +
                             std::get<GlyphBlock>(op).glyphs;
                     ++i) {
                    hb_glyph_info_t &_info = glyphInfo[i];
                    hb_glyph_position_t &_pos = glyphPos[i];
                    GlyphData &glyph = ctx.engine->fonts.getGlyph(
                        font, _info.codepoint, std::round(size * glyphScale));
                    size_t txtPointer = _info.cluster;
                    while (it.index() < txtPointer) {
                        ++it;
                    }

                    GlyphRenderData renderData(
                        _info.codepoint, color,
                        scale, // FIXME
                        Point((cursor.x() + _pos.x_offset) * fontScale,
                              (cursor.y() - _pos.y_offset) * fontScale));
                    if (custom) {
                        custom(&ctx, this, &renderData);
                    }
                    float fullScaleX = cameraScale / renderData.scale.x(),
                          fullScaleY = cameraScale / renderData.scale.y();
                    Matrix4 matrix;
                    matrix.identity();
                    DrawKey key;
                    if (pixelPerfect) {
                        key.smooth = SmoothingMode::SmoothNearest;
                        matrix.translate(position.x(), position.y());
                        matrix.tx() =
                            std::round(matrix.tx() * fullScaleX) / fullScaleX;
                        matrix.ty() =
                            std::round(matrix.ty() * fullScaleY) / fullScaleY;
                        matrix.a() = 1.0 / glyphScale / fullScaleX;
                        matrix.d() = 1.0 / glyphScale / fullScaleY;
                        matrix.a() *=
                            ctx.camera->scale.x() / ctx.camera->scale.y();
                        matrix.b() = matrix.c() = 0;
                        matrix.translate(
                            std::round(renderData.position.x() * fullScaleX) /
                                fullScaleX,
                            std::round(renderData.position.y() * fullScaleY) /
                                fullScaleY);
                        matrix.translate(
                            std::round(glyph.offset.x() / glyphScale) /
                                fullScaleX,
                            std::round(-glyph.offset.y() / glyphScale) /
                                fullScaleY);
                    } else {
                        key.smooth = smooth == SmoothingMode::SmoothMipmap
                                         ? SmoothingMode::SmoothLinear
                                         : this->smooth;
                        matrix.translate(position.x(), position.y());
                        matrix.translate(renderData.position.x(),
                                         renderData.position.y());
                        matrix.a() = 1.0 / glyphScale / fullScaleX;
                        matrix.d() = 1.0 / glyphScale / fullScaleY;
                        matrix.a() *=
                            ctx.camera->scale.x() / ctx.camera->scale.y();
                        matrix.b() = matrix.c() = 0;
                        matrix.translate(
                            glyph.offset.x() / glyphScale / fullScaleX,
                            -glyph.offset.y() / glyphScale / fullScaleY);
                    }
                    key.image = glyph.region.img;
                    key.blend = blendMode;
                    key.shader =
                        this->shader
                            ? this->shader
                            : ctx.engine->renderer.getDefaultTextShader();
                    if (border) {
                        if (_borderEnabled) {
                            float thickness = borderThickness * cameraScale;
                            Color borderColor(this->borderColor.r,
                                              this->borderColor.g,
                                              this->borderColor.b,
                                              this->borderColor.a * color.a);
                            GlyphData &borderGlyph = ctx.engine->fonts.getGlyph(
                                font, _info.codepoint,
                                std::round(size * glyphScale),
                                std::round(thickness * glyphScale));
                            matrix.tx() +=
                                (borderGlyph.offset.x() - glyph.offset.x()) /
                                glyphScale / cameraScale;
                            matrix.ty() -=
                                (borderGlyph.offset.y() - glyph.offset.y()) /
                                glyphScale / cameraScale;
                            if (pitch) {
                                matrix.pitch(pitch);
                            }
                            ctx.addRect(key, borderGlyph.region.rect, matrix,
                                        borderColor, zIndex);
                        }
                    } else {
                        if (pitch) {
                            matrix.pitch(pitch);
                        }
                        ctx.addRect(key, glyph.region.rect, matrix,
                                    renderData.color, zIndex);
                    }

                    cursor.x() += _pos.x_advance * renderData.scale.x();
                    if (charCount > -1) {
                        charCount -= std::max(charDelays[*it], 1);
                        if (charCount <= 0) {
                            return;
                        }
                    }
                }
                break;
            }
            case Whitespace: {
                if (charCount > -1) {
                    --charCount;
                    if (charCount <= 0) {
                        return;
                    }
                }
                cursor.x() += std::get<Whitespace>(op);
                break;
            }
            case Tab: {
                cursor.x() = this->tabStops[tabIndex++];
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

void Text::setTabStops(const std::string &stops) {
    tabStops.clear();
    std::stringstream stream(stops);
    std::string token;
    while (getline(stream, token, ',')) {
        int stop = atoi(token.c_str());
        tabStops.push_back(stop);
    }
}

}
