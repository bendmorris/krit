#ifndef KRIT_SPRITES_TEXT
#define KRIT_SPRITES_TEXT

#include "krit/Sprite.h"
#include "krit/asset/Font.h"
#include "krit/math/Dimensions.h"
#include "krit/math/Point.h"
#include "krit/utils/Color.h"
#include <cassert>
#include <functional>
#include <optional>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct hb_buffer_t;
struct hb_font_t;

namespace krit {

struct Text;
struct RenderContext;

enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
};

struct NewlineData {
    Dimensions first;
    AlignType second;

    NewlineData(const Dimensions &d, AlignType a) : first(d), second(a) {}
};

struct GlyphRenderData {
    int32_t c = 0;
    Color color;
    Vec2f scale{1, 1};
    Point position;

    GlyphRenderData() {}
    GlyphRenderData(const Point &position) : position(position) {}
    GlyphRenderData(int32_t c, Color color, Vec2f &scale, const Point &position)
        : c(c), color(color), scale(scale), position(position) {}
};

using CustomTextRenderFunction =
    std::function<void(RenderContext *, Text *, GlyphRenderData *)>;

struct TextOptions {
    static TextOptions *create() { return new TextOptions(); }

    std::shared_ptr<Font> font;
    int size = 16;
    AlignType align = LeftAlign;
    bool wordWrap = false;
    float lineSpacing = 0;
    float charSpacing = 0;

    TextOptions() {}

    TextOptions &setFontAsset(std::shared_ptr<Font> font) {
        this->font = font;
        return *this;
    }
    TextOptions &setFont(const std::string &name);
    TextOptions &setSize(int size) {
        this->size = size;
        return *this;
    }
    TextOptions &setAlign(AlignType align) {
        this->align = align;
        return *this;
    }
    TextOptions &setWordWrap(bool wrap) {
        this->wordWrap = wrap;
        return *this;
    }
    TextOptions &setLineSpacing(float spacing) {
        this->lineSpacing = spacing;
        return *this;
    }
    TextOptions &setCharSpacing(float spacing) {
        this->charSpacing = spacing;
        return *this;
    }
};

struct TextFormatTagOptions {
    static TextFormatTagOptions *create() { return new TextFormatTagOptions(); }

    std::optional<Color> color;
    std::optional<AlignType> align;
    bool newline = false;
    bool tab = false;
    bool border = false;
    int charDelay = 0;
    CustomTextRenderFunction custom;
    VisibleSprite *sprite = nullptr;
    std::shared_ptr<Font> font;

    TextFormatTagOptions() = default;

    TextFormatTagOptions &setFont(const std::string &fontName);
    TextFormatTagOptions &setColor(uint32_t c) {
        this->color = Color(c, 1.0);
        return *this;
    }
    TextFormatTagOptions &setAlign(AlignType a) {
        this->align = a;
        return *this;
    }
    TextFormatTagOptions &setNewline() {
        this->newline = true;
        return *this;
    }
    TextFormatTagOptions &setTab() {
        this->tab = true;
        return *this;
    }
    TextFormatTagOptions &setCustom(CustomTextRenderFunction c) {
        this->custom = c;
        return *this;
    }
    TextFormatTagOptions &setSprite(VisibleSprite *s) {
        this->sprite = s;
        return *this;
    }
    TextFormatTagOptions &setDelay(int delay) {
        this->charDelay = delay;
        return *this;
    }
    TextFormatTagOptions &setBorder() {
        this->border = true;
        return *this;
    }
};

struct GlyphBlockData {
    size_t startIndex;
    size_t glyphs;
    float trailingWhitespace;

    GlyphBlockData(size_t a, size_t b, float c)
        : startIndex(a), glyphs(b), trailingWhitespace(c) {}
};

struct TextRunData {
    std::shared_ptr<Font> font;
    hb_buffer_t *hbBuf;
    int lineHeight = 0;
};

enum TextOpcodeType : int {
    // parse time only
    RawText,
    // render time only
    StartTextRun,
    // general
    SetFont,
    SetColor,
    SetAlign,
    SetCustom,
    GlyphBlock,
    NewLine,
    RenderSprite,
    Tab,
    Whitespace,
    CharDelay,
    EnableBorder,
    DisableBorder,
};

using TextOpcode =
    std::variant<std::pair<size_t, size_t>, TextRunData, std::shared_ptr<Font>, Color, AlignType,
                 CustomTextRenderFunction, GlyphBlockData, NewlineData,
                 VisibleSprite *, std::monostate, float, int, std::monostate,
                 std::monostate>;

struct Text : public VisibleSprite, public TextOptions {
    static std::unordered_map<std::string, TextFormatTagOptions> formatTags;

    static void addFormatTag(std::string, TextFormatTagOptions);
    static Text *create(const TextOptions &options) {
        return new Text(options);
    }

    float charCount = -1;
    int maxChars = 0;
    std::vector<float> tabStops;
    Color baseColor = Color::white();
    std::string text;
    Dimensions textDimensions;
    bool allowPixelPerfect = false;
    bool dynamicSize = true;
    bool border = false;
    int borderThickness = 0;
    float glyphScale = 1;
    Color borderColor = Color::black();
    float pitch = 0;

    Text() = default;
    Text(const TextOptions &options);
    Text(const Text &other) {
        *this = other;
        this->hbBuf = nullptr;
        this->dirty = true;
    }
    virtual ~Text();

    Text &setText(const std::string &text);
    Text &setRichText(const std::string &text);
    Text &refresh();
    void invalidate() { dirty = true; }

    const std::string &getText() { return this->text; }
    const Dimensions &getTextDimensions() {
        refresh();
        return this->textDimensions;
    }

    Dimensions getSize() override {
        this->refresh();
        return textDimensions;
    }
    float &width() {
        refresh();
        return textDimensions.x();
    }
    float &height() {
        refresh();
        return textDimensions.y();
    }
    void setFontSize(int size) {
        if (this->size != size) {
            this->dirty = true;
            this->size = size;
        }
    }
    void resize(float, float) override;
    void render(RenderContext &ctx) override;

    void setTabStops(const std::string &stops);

private:
    std::vector<TextOpcode> opcodes;
    hb_buffer_t *hbBuf = nullptr;
    bool rich = false;
    bool dirty = false;
    Dimensions renderedSize;
    bool hasBorderTags = false;

    friend struct TextParser;

    void __render(RenderContext &ctx, bool border);
};

}

#endif
