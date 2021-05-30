#ifndef KRIT_SPRITES_TEXT
#define KRIT_SPRITES_TEXT

#include "krit/asset/Font.h"
#include "krit/sprites/TextBase.h"
#include "krit/utils/Option.h"
#include "krit/Sprite.h"

struct hb_buffer_t;
struct hb_font_t;

namespace krit {

struct Text;

typedef void CustomTextRenderFunction(RenderContext*, Text*, GlyphRenderData*);

struct TextOptions {
    Font *font;
    int size = 16;
    AlignType align = LeftAlign;
    bool wordWrap = false;
    double lineSpacing = 0;

    TextOptions() {}

    TextOptions &setFont(Font *font) { this->font = font; return *this; }
    TextOptions &setFont(const std::string &name) { this->font = Font::getFont(name); return *this; }
    TextOptions &setSize(int size) { this->size = size; return *this; }
    TextOptions &setAlign(AlignType align) { this->align = align; return *this; }
    TextOptions &setWordWrap(bool wrap) { this->wordWrap = wrap; return *this; }
    TextOptions &setLineSpacing(double spacing) { this->lineSpacing = spacing; return *this; }
};

struct TextFormatTagOptions {
    Option<Color> color;
    Option<AlignType> align;
    bool newline = false;
    bool tab = false;
    CustomTextRenderFunction *custom = nullptr;
    VisibleSprite *sprite = nullptr;

    TextFormatTagOptions() = default;

    TextFormatTagOptions &setColor(Color c) { this->color = Option<Color>(c); return *this; }
    TextFormatTagOptions &setAlign(AlignType a) { this->align = Option<AlignType>(a); return *this; }
    TextFormatTagOptions &setNewline() { this->newline = true; return *this; }
    TextFormatTagOptions &setTab() { this->tab = true; return *this; }
    TextFormatTagOptions &setCustom(CustomTextRenderFunction *c) { this->custom = c; return *this; }
    TextFormatTagOptions &setSprite(VisibleSprite *s) { this->sprite = s; return *this; }
};

struct TextGlyphData {
    uint32_t codepoint;
    int32_t xOffset, yOffset, xAdvance, yAdvance;
};

union TextOpcodeData {
    bool present;
    Color color;
    double number;
    AlignType align;
    CustomTextRenderFunction *custom = nullptr;
    std::pair<size_t, size_t> glyphBlock;
    NewlineData newLine;
    VisibleSprite *sprite;

    TextOpcodeData(): present(false) {}
    TextOpcodeData(Color color): color(color) {}
    TextOpcodeData(double number): number(number) {}
    TextOpcodeData(AlignType align): align(align) {}
    TextOpcodeData(CustomTextRenderFunction *custom): custom(custom) {}
    TextOpcodeData(size_t startIndex, size_t glyphs): glyphBlock(startIndex, glyphs) {}
    TextOpcodeData(Dimensions d, AlignType a): newLine(d, a) {}
    TextOpcodeData(VisibleSprite *sprite): sprite(sprite) {}
};

enum TextOpcodeType: int;

struct TextOpcode {
    TextOpcodeType type;
    TextOpcodeData data;

    TextOpcode() = default;
    TextOpcode(TextOpcodeType type, TextOpcodeData data): type(type), data(data) {}
    TextOpcode(TextOpcodeType type): type(type) {}
};

struct Text: public VisibleSprite {
    static std::unordered_map<std::string, TextFormatTagOptions> formatTags;

    static void addFormatTag(std::string, TextFormatTagOptions);

    TextOptions options;
    int charCount = -1;
    int maxChars = 0;
    std::vector<double> tabStops;
    Color baseColor = Color::white();

    Text() = default;
    Text(const TextOptions &options);
    virtual ~Text();

    Text &setText(const std::string &text);
    Text &setRichText(const std::string &text);
    Text &refresh();

    const std::string &getText() { return this->text; }
    const Dimensions &getTextDimensions() { refresh(); return this->textDimensions; }

    Dimensions getSize() override { this->refresh(); return textDimensions; }
    double width() { refresh(); return textDimensions.width(); }
    double height() { refresh(); return textDimensions.height(); }
    void resize(double, double) override;
    void render(RenderContext &ctx) override;

    private:
        std::string text;
        std::vector<TextOpcode> opcodes;
        Font *font = nullptr;
        hb_buffer_t *hbBuf = nullptr;
        bool rich = false;
        bool dirty = false;
        Dimensions textDimensions;
        friend struct TextParser;
};

}

#endif
