#include "krit/sprites/Text.h"
#include "harfbuzz/hb.h"
#include "krit/Camera.h"
#include "krit/Engine.h"
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
#include "Tracy.hpp"
#endif

namespace krit {

static const int FONT_SCALE = 64;

static std::vector<hb_buffer_t *> recycledBuffers;
hb_buffer_t *getHbBuffer() {
    hb_buffer_t *hbBuf;
    if (recycledBuffers.empty()) {
        hbBuf = hb_buffer_create();
        hb_buffer_set_direction(hbBuf, HB_DIRECTION_LTR);
        hb_buffer_set_script(hbBuf, HB_SCRIPT_LATIN);
        hb_buffer_set_language(hbBuf, hb_language_from_string("en", -1));
        hb_buffer_set_cluster_level(
            hbBuf, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
    } else {
        hbBuf = recycledBuffers.back();
        recycledBuffers.pop_back();
    }
    return hbBuf;
}
void recycleHbBuffer(hb_buffer_t *buf) { recycledBuffers.push_back(buf); }

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
    std::stack<std::shared_ptr<Font>> font;
    std::stack<Color> color;
    std::stack<AlignType> align;
    std::stack<CustomTextRenderFunction> custom;

    TextRenderStack() {}

    void clear() {
        clearStack(this->font);
        clearStack(this->color);
        clearStack(this->align);
        clearStack(this->custom);
    }
};

static std::unordered_map<hb_codepoint_t, int> charDelays = {
    {'.', 10}, {',', 6}, {'!', 10}, {'?', 10}, {'-', 6}, {';', 4}, {':', 4},
};

static void debugPrint(const std::vector<TextOpcode> &ops) {
    for (auto &op : ops) {
        switch (op.index()) {
            case RawText: {
                LOG_DEBUG("RawText");
                break;
            }
            case StartTextRun: {
                LOG_DEBUG("StartTextRun");
                break;
            }
            case SetColor: {
                LOG_DEBUG("SetColor");
                break;
            }
            case SetAlign: {
                LOG_DEBUG("SetAlign");
                break;
            }
            case SetCustom: {
                LOG_DEBUG("SetCustom");
                break;
            }
            case GlyphBlock: {
                LOG_DEBUG("GlyphBlock");
                break;
            }
            case NewLine: {
                LOG_DEBUG("NewLine");
                break;
            }
            case RenderSprite: {
                LOG_DEBUG("RenderSprite");
                break;
            }
            case Tab: {
                LOG_DEBUG("Tab");
                break;
            }
            case Whitespace: {
                LOG_DEBUG("Whitespace");
                break;
            }
            case CharDelay: {
                LOG_DEBUG("CharDelay");
                break;
            }
            case EnableBorder: {
                LOG_DEBUG("EnableBorder");
                break;
            }
            case DisableBorder: {
                LOG_DEBUG("DisableBorder");
                break;
            }
        }
    }
}

struct FormatTag {
    std::string_view name;
    std::vector<std::pair<std::string_view, std::string_view>> attributes;
};

struct TextRun {
    std::shared_ptr<Font> font;
    std::string rawText;
    hb_buffer_t *hbBuf = nullptr;
    std::vector<TextOpcode> opcodes;

    TextRun(std::shared_ptr<Font> font) : font(font) { hbBuf = getHbBuffer(); }
};

/**
 * Utility structure to parse format tags from a string of text and generate a
 * Vector of TextOpcodes.
 */
struct TextParser {
    static TextRenderStack stack;

    Point cursor;

    AlignType currentAlign = LeftAlign;
    int newLineIndex = -1;
    size_t tabIndex = 0;

    std::vector<TextOpcode> word;
    int wordLength = 0;
    int lineHeight = 0;

    hb_buffer_t *hbBuf;

    void parseText(Text &txt, const std::string &s, bool rich) {
#if TRACY_ENABLE
        ZoneScopedN("TextParser::parseText");
#endif
        txt.opcodes.clear();
        txt.textDimensions.setTo(0, 0);
        txt.maxChars = 0;
        cursor.setTo(0, 0);

        if (s.empty()) {
            return;
        }

        // pass 1: find all format tags, and convert the text into TextRuns
        std::vector<TextRun> runs;
        runs.emplace_back(txt.font);
        TextRenderStack &st = TextParser::stack;
        st.clear();
        st.font.push(txt.font);
        st.color.push(txt.baseColor);
        st.align.push(this->currentAlign = txt.align);
        st.custom.push(nullptr);
        word.clear();

        int rawTextStart = -1;

        for (Utf8Iterator it = s.begin(); it != s.end(); ++it) {
            size_t i = it.index();
            char32_t c = *it;
            if (c == '<' || c == '\n' || c == '\t') {
                if (rawTextStart > -1) {
                    size_t originalSize = runs.back().rawText.size();
                    size_t begin = rawTextStart;
                    size_t length = it.index() - begin;
                    runs.back().rawText.append(s.c_str() + begin, length);
                    runs.back().opcodes.emplace_back(
                        std::in_place_index_t<RawText>(), originalSize, length);
                    rawTextStart = -1;
                }
                switch (c) {
                    case '\n': {
                        runs.back().opcodes.emplace_back(
                            TextOpcode(std::in_place_index_t<NewLine>(),
                                       Dimensions(), LeftAlign));
                        break;
                    }
                    case '\t': {
                        runs.back().opcodes.emplace_back(
                            TextOpcode(std::in_place_index_t<Tab>()));
                        break;
                    }
                    default: {
                        int startPos = i;
                        int endPos = it.index() + 1;
                        if (c == '<') {
                            // we have a tag; look for the end
                            // FIXME: also parse attributes here
                            for (; it != s.end(); ++it) {
                                endPos = it.index();
                                char32_t c2 = *it;
                                if (c2 == '>') {
                                    break;
                                }
                            }
                        }
                        // identify the tag and convert it into ops
                        std::string_view tagName(s.c_str() + startPos + 1,
                                                 endPos - startPos - 1);
                        bool open = true;
                        bool close = false;
                        if (tagName[0] == '/') {
                            // close
                            open = false;
                            close = true;
                            tagName = std::string_view(&tagName[1],
                                                       tagName.length() - 1);
                        } else if (tagName[tagName.length() - 1] == '/') {
                            // self closing
                            open = close = true;
                            tagName = std::string_view(&tagName[0],
                                                       tagName.length() - 1);
                        }
                        static std::string tagStr;
                        tagStr.assign(tagName.data(), tagName.length());
                        // printf("TAG: <%s> %s%s\n", tagStr.c_str(),
                        //        open ? "OPEN" : "", close ? "CLOSE" : "");
                        auto found = Text::formatTags.find(tagStr);
                        if (found != Text::formatTags.end()) {
                            TextFormatTagOptions &tag = found->second;
                            if (tag.color) {
                                if (open) {
                                    st.color.push(*tag.color);
                                }
                                if (close) {
                                    st.color.pop();
                                }
                                runs.back().opcodes.emplace_back(TextOpcode(
                                    std::in_place_index_t<SetColor>(),
                                    st.color.top()));
                            }
                            if (tag.align) {
                                if (open) {
                                    st.align.push(*tag.align);
                                }
                                if (close) {
                                    st.align.pop();
                                }
                                runs.back().opcodes.emplace_back(TextOpcode(
                                    std::in_place_index_t<SetAlign>(),
                                    currentAlign = st.align.top()));
                            }
                            if (tag.custom != nullptr) {
                                if (open) {
                                    st.custom.push(tag.custom);
                                }
                                if (close) {
                                    st.custom.pop();
                                }
                                runs.back().opcodes.emplace_back(TextOpcode(
                                    std::in_place_index_t<SetCustom>(),
                                    st.custom.top()));
                            }
                            if (tag.sprite && open) {
                                runs.back().opcodes.emplace_back(TextOpcode(
                                    std::in_place_index_t<RenderSprite>(),
                                    tag.sprite));
                            }
                            if (tag.newline && open) {
                                runs.back().opcodes.emplace_back(
                                    TextOpcode(std::in_place_index_t<NewLine>(),
                                               Dimensions(), LeftAlign));
                            }
                            if (tag.tab && open) {
                                runs.back().opcodes.emplace_back(
                                    TextOpcode(std::in_place_index_t<Tab>()));
                            }
                            if (tag.charDelay && !close) {
                                runs.back().opcodes.emplace_back(TextOpcode(
                                    std::in_place_index_t<CharDelay>(),
                                    tag.charDelay));
                            }
                            if (tag.border) {
                                if (open) {
                                    runs.back().opcodes.emplace_back(TextOpcode(
                                        std::in_place_index_t<EnableBorder>()));
                                }
                                if (close) {
                                    runs.back().opcodes.emplace_back(
                                        TextOpcode(std::in_place_index_t<
                                                   DisableBorder>()));
                                }
                            }
                            if (tag.font) {
                                // this will begin a new run
                                if (open) {
                                    st.font.push(tag.font);
                                }
                                if (close) {
                                    st.font.pop();
                                }
                                if (runs.back().font != st.font.top()) {
                                    runs.emplace_back(st.font.top());
                                }
                            }
                        }
                    }
                }
            } else {
                // part of a raw text run
                if (rawTextStart == -1) {
                    rawTextStart = it.index();
                }
            }
        }
        if (rawTextStart > -1) {
            size_t originalSize = runs.back().rawText.size();
            size_t begin = rawTextStart;
            size_t length = s.length() - begin;
            runs.back().rawText.append(s.c_str() + begin, length);
            runs.back().opcodes.emplace_back(std::in_place_index_t<RawText>(),
                                             originalSize, length);
        }

        // pass 2: shape and layout
        size_t glyphCost = 0;
        for (size_t i = 0; i < runs.size(); ++i) {
            auto &run = runs[i];
            // shape the text
            auto &hbBuf = run.hbBuf;
            auto &rawText = run.rawText;
            hb_buffer_add_utf8(hbBuf, rawText.c_str(), rawText.size(), 0, -1);
            run.font->shape(hbBuf);
            Utf8Iterator it = rawText.begin();
            unsigned int glyphCount;
            hb_glyph_info_t *glyphInfo =
                hb_buffer_get_glyph_infos(hbBuf, &glyphCount);
            hb_glyph_position_t *glyphPos =
                hb_buffer_get_glyph_positions(hbBuf, &glyphCount);

            size_t glyphCursor = 0;

            hb_font_extents_t extents;
            hb_font_get_h_extents(run.font->font, &extents);
            lineHeight = (extents.ascender - extents.descender +
                          extents.line_gap); // * txt.size / FONT_SCALE;

            this->addOp(txt,
                        TextOpcode(std::in_place_index_t<StartTextRun>(),
                                   (TextRunData){.font = run.font,
                                                 .hbBuf = run.hbBuf,
                                                 .lineHeight = lineHeight}));

            for (auto &op : run.opcodes) {
                switch (op.index()) {
                    case RawText: {
                        size_t start = std::get<RawText>(op).first;
                        size_t end = start + std::get<RawText>(op).second;
                        for (; glyphCursor < glyphCount; ++glyphCursor) {
                            hb_glyph_info_t &_glyphInfo =
                                glyphInfo[glyphCursor];
                            hb_glyph_position_t &_glyphPos =
                                glyphPos[glyphCursor];
                            size_t txtPointer = _glyphInfo.cluster;
                            if (txtPointer >= end) {
                                break;
                            }
                            while (it.index() < txtPointer) {
                                ++it;
                            }
                            char32_t c = *it;
                            switch (c) {
                                case ' ': {
                                    flushWord(txt);
                                    cursor.x() += _glyphPos.x_advance;
                                    if (txt.opcodes.back().index() ==
                                        Whitespace) {
                                        std::get<Whitespace>(
                                            txt.opcodes.back()) +=
                                            _glyphPos.x_advance;
                                    } else {
                                        txt.opcodes.emplace_back(
                                            std::in_place_index_t<Whitespace>(),
                                            _glyphPos.x_advance);
                                    }
                                    break;
                                }
                                default: {
                                    glyphCost +=
                                        1; // std::max(charDelays[c], 1);
                                    if (!word.empty() &&
                                        word.back().index() == GlyphBlock &&
                                        std::get<GlyphBlock>(word.back())
                                                    .startIndex +
                                                std::get<GlyphBlock>(
                                                    word.back())
                                                    .glyphs ==
                                            glyphCursor) {
                                        auto &back = word.back();
                                        // add this glyph to the existing opcode
                                        ++std::get<GlyphBlock>(back).glyphs;
                                        wordLength += _glyphPos.x_advance;
                                        // cursor.x += _glyphPos.x_advance;
                                    } else {
                                        word.emplace_back(
                                            std::in_place_index_t<GlyphBlock>(),
                                            glyphCursor, 1, 0);
                                        wordLength += _glyphPos.x_advance;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default: {
                        this->addOp(txt, op);
                    }
                }
            }
        }
        newLine(txt);

        if (txt.wordWrap) {
            txt.renderedSize.copyFrom(txt.dimensions);
            txt.dimensions.y() = txt.textDimensions.y();
        } else {
            txt.dimensions.copyFrom(txt.textDimensions);
        }

        txt.maxChars = glyphCost;
        for (auto it : txt.opcodes) {
            if (it.index() == CharDelay) {
                txt.maxChars += std::get<CharDelay>(it);
            } else if (it.index() == Whitespace) {
                ++txt.maxChars;
            }
        }

        if (krit::Log::level <= LogLevel::Info) {
            debugPrint(txt.opcodes);
        }
    }

    void addInitialNewline(Text &txt) {
        if (newLineIndex == -1) {
            newLineIndex = txt.opcodes.size();
            txt.opcodes.emplace_back(std::in_place_index_t<NewLine>(),
                                     Dimensions(), txt.align);
        }
    }

    void flushWord(Text &txt) {
        if (!word.empty()) {
            addInitialNewline(txt);
        }
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

    void newLine(Text &txt, bool canBreak = true) {
        if (canBreak) {
            this->flushWord(txt);
        }
        float trailingWhitespace = 0;
        if (txt.opcodes.back().index() == Whitespace) {
            trailingWhitespace = std::get<Whitespace>(txt.opcodes.back());
        }
        addInitialNewline(txt);
        if (txt.opcodes[this->newLineIndex].index() == NewLine) {
            // std::pair<Dimensions, AlignType> &align =
            // txt.opcodes[this->newLineIndex].data.newLine; update the size of
            // the preceding line
            float add = this->newLineIndex == 0 ? 0 : txt.lineSpacing;
            this->cursor.y() += lineHeight + add;
            std::get<NewLine>(txt.opcodes[this->newLineIndex])
                .first.setTo(this->cursor.x() - trailingWhitespace,
                             lineHeight + add);
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
            case StartTextRun: {
                flushWord(txt);
                txt.opcodes.push_back(op);
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
                this->currentAlign = v;
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
                // fallthrough
            }
            default: {
                word.push_back(op);
            }
        }
    }
};

TextRenderStack TextParser::stack;

TextFormatTagOptions &TextFormatTagOptions::setFont(const std::string &name) {
    this->font = engine->fonts.getFont(name);
    assert(this->font);
    return *this;
}

TextOptions &TextOptions::setFont(const std::string &name) {
    this->font = engine->fonts.getFont(name);
    assert(this->font);
    return *this;
}

Text::Text(const TextOptions &options) : TextOptions(options) { assert(font); }

Text::~Text() {
    if (hbBuf) {
        hb_buffer_clear_contents(hbBuf);
        recycleHbBuffer(hbBuf);
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
    float fontScale = size / cameraScale / 64.0;
    bool pixelPerfect = allowPixelPerfect && fontScale < 20;

    hb_buffer_t *hbBuf = nullptr;
    std::shared_ptr<Font> font = this->font;
    int lineHeight = 0;

    for (TextOpcode &op : this->opcodes) {
        switch (op.index()) {
            case StartTextRun: {
                hbBuf = std::get<StartTextRun>(op).hbBuf;
                font = std::get<StartTextRun>(op).font;
                lineHeight = std::get<StartTextRun>(op).lineHeight;
                break;
            }
            case SetFont: {
                font = std::get<SetFont>(op);
                break;
            }
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
                float align = 0;
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
                        this->position.x() +
                            renderData.position.x() * fontScale,
                        this->position.y() +
                            renderData.position.y() * fontScale +
                            (lineHeight * scale.y() * fontScale - size.y()) -
                            lineHeight * fontScale,
                        this->position.z());
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
                    GlyphData &glyph =
                        engine->fonts.getGlyph(font.get(), _info.codepoint,
                                               std::round(size * glyphScale));
                    // size_t txtPointer = _info.cluster;

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
                        matrix.translate(position.x(), position.y(),
                                         position.z());
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
                        matrix.translate(position.x() + renderData.position.x(),
                                         position.y() + renderData.position.y(),
                                         position.z());
                        matrix.a() = ctx.camera->scale.x() /
                                     ctx.camera->scale.y() / glyphScale /
                                     fullScaleX;
                        matrix.d() = 1.0 / glyphScale / fullScaleY;
                        matrix.b() = matrix.c() = 0;
                        matrix.translate(
                            glyph.offset.x() / glyphScale / fullScaleX,
                            -glyph.offset.y() / glyphScale / fullScaleY);
                    }
                    key.image = glyph.region.img;
                    key.blend = blendMode;
                    key.shader = this->shader
                                     ? this->shader
                                     : engine->renderer.getDefaultTextShader();
                    if (border) {
                        if (_borderEnabled) {
                            float thickness = borderThickness * cameraScale;
                            Color borderColor(this->borderColor.r,
                                              this->borderColor.g,
                                              this->borderColor.b,
                                              this->borderColor.a * color.a);
                            GlyphData &borderGlyph = engine->fonts.getGlyph(
                                font.get(), _info.codepoint,
                                std::round(size * glyphScale),
                                std::round(thickness * glyphScale));
                            matrix.tx() +=
                                (borderGlyph.offset.x() - glyph.offset.x()) /
                                glyphScale / fullScaleX;
                            matrix.ty() -=
                                (borderGlyph.offset.y() - glyph.offset.y()) /
                                glyphScale / fullScaleY;
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
                        // FIXME: charDelays
                        --charCount; //   -= std::max(charDelays[*it], 1);
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
