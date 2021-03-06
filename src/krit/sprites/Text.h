#ifndef KRIT_SPRITES_TEXT
#define KRIT_SPRITES_TEXT

#include <cassert>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "krit/Sprite.h"
#include "krit/asset/Font.h"
#include "krit/math/Dimensions.h"
#include "krit/sprites/TextBase.h"
#include "krit/utils/Color.h"
#include "krit/utils/Option.h"

struct hb_buffer_t;
struct hb_font_t;

namespace krit {

struct Text;
struct RenderContext;

typedef void CustomTextRenderFunction(RenderContext *, Text *,
                                      GlyphRenderData *);

struct TextOptions {
    Font *font = nullptr;
    int size = 16;
    AlignType align = LeftAlign;
    bool wordWrap = false;
    float lineSpacing = 0;

    TextOptions() {}

    TextOptions &setFont(Font *font) {
        this->font = font;
        return *this;
    }
    TextOptions &setFont(const std::string &name) {
        this->font = Font::getFont(name);
        assert(this->font);
        return *this;
    }
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
};

struct TextFormatTagOptions {
    Option<Color> color;
    Option<AlignType> align;
    bool newline = false;
    bool tab = false;
    CustomTextRenderFunction *custom = nullptr;
    VisibleSprite *sprite = nullptr;

    TextFormatTagOptions() = default;

    TextFormatTagOptions &setColor(Color c) {
        this->color = Option<Color>(c);
        return *this;
    }
    TextFormatTagOptions &setAlign(AlignType a) {
        this->align = Option<AlignType>(a);
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
    TextFormatTagOptions &setCustom(CustomTextRenderFunction *c) {
        this->custom = c;
        return *this;
    }
    TextFormatTagOptions &setSprite(VisibleSprite *s) {
        this->sprite = s;
        return *this;
    }
};

struct GlyphBlockData {
    size_t startIndex;
    size_t glyphs;
    float trailingWhitespace;
};

union TextOpcodeData {
    bool present;
    Color color;
    float number;
    AlignType align;
    CustomTextRenderFunction *custom = nullptr;
    GlyphBlockData glyphBlock;
    NewlineData newLine;
    VisibleSprite *sprite;

    TextOpcodeData() : present(false) {}
    TextOpcodeData(Color color) : color(color) {}
    TextOpcodeData(float number) : number(number) {}
    TextOpcodeData(AlignType align) : align(align) {}
    TextOpcodeData(CustomTextRenderFunction *custom) : custom(custom) {}
    TextOpcodeData(size_t startIndex, size_t glyphs, float trailingWhitespace)
        : glyphBlock{.startIndex = startIndex,
                     .glyphs = glyphs,
                     .trailingWhitespace = trailingWhitespace} {}
    TextOpcodeData(Dimensions d, AlignType a) : newLine(d, a) {}
    TextOpcodeData(VisibleSprite *sprite) : sprite(sprite) {}
};

enum TextOpcodeType : int;

struct TextOpcode {
    TextOpcodeType type;
    TextOpcodeData data;

    TextOpcode() = default;
    TextOpcode(TextOpcodeType type, TextOpcodeData data)
        : type(type), data(data) {}
    TextOpcode(TextOpcodeType type) : type(type) {}

    void debugPrint();
};

struct Text : public VisibleSprite, public TextOptions {
    static std::unordered_map<std::string, TextFormatTagOptions> formatTags;

    static void addFormatTag(std::string, TextFormatTagOptions);

    int charCount = -1;
    int maxChars = 0;
    std::vector<float> tabStops;
    Color baseColor = Color::white();
    std::string text;
    Dimensions textDimensions;

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

    const std::string &getText() { return this->text; }
    const Dimensions &getTextDimensions() {
        refresh();
        return this->textDimensions;
    }

    Dimensions getSize() override {
        this->refresh();
        return textDimensions;
    }
    float width() {
        refresh();
        return textDimensions.width();
    }
    float height() {
        refresh();
        return textDimensions.height();
    }
    void resize(float, float) override;
    void render(RenderContext &ctx) override;

private:
    std::vector<TextOpcode> opcodes;
    hb_buffer_t *hbBuf = nullptr;
    bool rich = false;
    bool dirty = false;
    friend struct TextParser;
    float lineHeight;
};

}

#endif
