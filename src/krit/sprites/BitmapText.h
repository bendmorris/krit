#ifndef KRIT_SPRITES_BITMAPTEXT
#define KRIT_SPRITES_BITMAPTEXT

#include "krit/asset/AssetContext.h"
#include "krit/asset/BitmapFont.h"
#include "krit/render/BlendMode.h"
#include "krit/utils/Color.h"
#include "krit/utils/Option.h"
#include "krit/utils/StringSlice.h"
#include "krit/Sprite.h"
#include "krit/Math.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace krit {

enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
};

struct BitmapTextOptions {
    std::shared_ptr<BitmapFont> font;
    int size = 16;
    AlignType align = LeftAlign;
    bool wordWrap = false;
    double lineSpacing = 0;
    bool dynamic = true;

    BitmapTextOptions() {}
    BitmapTextOptions(std::shared_ptr<BitmapFont> font): font(font) {}

    BitmapTextOptions &setSize(int size) { this->size = size; return *this; }
    BitmapTextOptions &setAlign(AlignType align) { this->align = align; return *this; }
    BitmapTextOptions &setWordWrap(bool wrap) { this->wordWrap = wrap; return *this; }
    BitmapTextOptions &setLineSpacing(double spacing) { this->lineSpacing = spacing; return *this; }
};

struct GlyphRenderData {
    char c = 0;
    Color color;
    ScaleFactor scale;
    Point position;

    GlyphRenderData() {}
    GlyphRenderData(const Point &position): position(position) {}
    GlyphRenderData(char c, Color color, ScaleFactor &scale, const Point &position):
        c(c), color(color), scale(scale), position(position) {}
};

class BitmapText;
typedef void CustomRenderFunction(RenderContext*, BitmapText*, GlyphRenderData*);

enum TextOpcodeType {
    SetColor,
    SetScale,
    SetAlign,
    SetCustom,
    SetFont,
    PopColor,
    PopScale,
    PopAlign,
    PopCustom,
    PopFont,
    TextBlock,
    NewLine,
    RenderSprite,
};

struct NewlineData {
    Dimensions first;
    AlignType second;

    NewlineData(const Dimensions &d, AlignType a): first(d), second(a) {}
};

union TextOpcodeData {
    bool present;
    Color color;
    double number;
    AlignType align;
    CustomRenderFunction *custom = nullptr;
    StringSlice text;
    NewlineData newLine;
    VisibleSprite *sprite;
    BitmapFont *font;

    TextOpcodeData(): present(false) {}
    TextOpcodeData(Color color): color(color) {}
    TextOpcodeData(double number): number(number) {}
    TextOpcodeData(AlignType align): align(align) {}
    TextOpcodeData(CustomRenderFunction *custom): custom(custom) {}
    TextOpcodeData(StringSlice text): text(text) {}
    TextOpcodeData(Dimensions d, AlignType a): newLine(d, a) {}
    TextOpcodeData(VisibleSprite *sprite): sprite(sprite) {}
    TextOpcodeData(BitmapFont *font): font(font) {}
};

struct TextOpcode {
    TextOpcodeType type;
    TextOpcodeData data;

    TextOpcode() = default;
    TextOpcode(TextOpcodeType type, TextOpcodeData data): type(type), data(data) {}
    TextOpcode(TextOpcodeType type): type(type) {}
};

struct FormatTagOptions {
    Option<Color> color;
    Option<double> scale;
    Option<AlignType> align;
    bool newline = false;
    CustomRenderFunction *custom = nullptr;
    VisibleSprite *sprite = nullptr;
    BitmapFont *font = nullptr;

    FormatTagOptions() = default;

    FormatTagOptions &setColor(Color c) { this->color = Option<Color>(c); return *this; }
    FormatTagOptions &setScale(double s) { this->scale = Option<double>(s); return *this; }
    FormatTagOptions &setAlign(AlignType a) { this->align = Option<AlignType>(a); return *this; }
    FormatTagOptions &setNewline() { this->newline = true; return *this; }
    FormatTagOptions &setCustom(CustomRenderFunction *c) { this->custom = c; return *this; }
    FormatTagOptions &setSprite(VisibleSprite *s) { this->sprite = s; return *this; }
    FormatTagOptions &setFont(BitmapFont *f) { this->font = f; return *this; }
};

struct TextParser;

/**
 * Text using bitmap fonts, supporting format tags to change the size, color,
 * etc.
 */
struct BitmapText: public VisibleSprite {
    static std::unordered_map<std::string, FormatTagOptions> formatTags;

    static void addFormatTag(std::string, FormatTagOptions);

    std::string text;
    Dimensions textDimensions;
    std::shared_ptr<BitmapFont> font;
    BitmapTextOptions options;
    int charCount = -1;
    int maxChars = 0;

    BitmapText() = default;
    BitmapText(const BitmapTextOptions &options);

    BitmapText &setFont(std::shared_ptr<BitmapFont> font);
    BitmapText &setText(const std::string &text);
    BitmapText &setRichText(const std::string &text);
    BitmapText &refresh();

    double baseScale() { return static_cast<double>(this->options.size) / this->font->size; }
    Dimensions getSize() override {
        this->refresh();
        double s = this->baseScale();
        return Dimensions(
            this->textDimensions.width() * s * scale.x,
            this->textDimensions.height() * s * scale.y
        );
    }
    double width() { this->refresh(); double s = this->baseScale(); return this->textDimensions.width() * s * scale.x; }
    double height() { this->refresh(); double s = this->baseScale(); return this->textDimensions.height() * s * scale.y; }
    void resize(double, double) override;
    void render(RenderContext &ctx) override;

    private:
        std::vector<TextOpcode> opcodes;
        bool rich = false;
        bool dirty = false;
        friend struct TextParser;
};

}

#endif
