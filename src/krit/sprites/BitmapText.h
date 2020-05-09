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
#include <vector>
#include <unordered_map>
#include <utility>

using namespace std;
using namespace krit;

namespace krit {

enum AlignType {
    LeftAlign,
    CenterAlign,
    RightAlign,
};

struct BitmapTextOptions {
    shared_ptr<BitmapFont> font;
    int size = 16;
    AlignType align = LeftAlign;
    bool wordWrap = false;
    double lineSpacing = 0;
    bool dynamic = true;

    BitmapTextOptions(shared_ptr<BitmapFont> font): font(font) {}

    BitmapTextOptions &setSize(int size) { this->size = size; return *this; }
    BitmapTextOptions &setAlign(AlignType align) { this->align = align; return *this; }
    BitmapTextOptions &setWordWrap(bool wrap) { this->wordWrap = wrap; return *this; }
    BitmapTextOptions &setLineSpacing(double spacing) { this->lineSpacing = spacing; return *this; }
};

struct GlyphRenderData {
    char c;
    Color color;
    ScaleFactor scale;
    Point position;

    GlyphRenderData() {}
    GlyphRenderData(char c, Color color, ScaleFactor &scale, Point position):
        c(c), color(color), scale(scale), position(position) {}
};

class BitmapText;
typedef void CustomRenderFunction(RenderContext*, BitmapText*, GlyphRenderData*);

enum TextOpcodeType {
    SetColor,
    SetScale,
    SetAlign,
    SetCustom,
    PopColor,
    PopScale,
    PopAlign,
    PopCustom,
    TextBlock,
    NewLine,
    InlineImage,
};

union TextOpcodeData {
    bool present;
    Color color;
    double number;
    AlignType align;
    CustomRenderFunction *custom;
    StringSlice text;
    std::pair<Dimensions, AlignType> newLine;
    ImageData *image;

    TextOpcodeData(): present(false) {}
    TextOpcodeData(Color color): color(color) {}
    TextOpcodeData(double number): number(number) {}
    TextOpcodeData(AlignType align): align(align) {}
    TextOpcodeData(CustomRenderFunction *custom): custom(custom) {}
    TextOpcodeData(StringSlice text): text(text) {}
    TextOpcodeData(Dimensions d, AlignType a): newLine(make_pair(d, a)) {}
    TextOpcodeData(std::shared_ptr<ImageData> image);
};

struct TextOpcode {
    TextOpcodeType type;
    TextOpcodeData data;

    TextOpcode(TextOpcodeType type, TextOpcodeData data): type(type), data(data) {}
    TextOpcode(TextOpcodeType type): type(type) {}
};

struct FormatTagOptions {
    Option<Color> color;
    Option<double> scale;
    Option<AlignType> align;
    bool newline = false;
    CustomRenderFunction *custom = nullptr;
    std::shared_ptr<ImageData> image = nullptr;

    FormatTagOptions() {}

    FormatTagOptions &setColor(Color c) { this->color = Option<Color>(c); return *this; }
    FormatTagOptions &setScale(double s) { this->scale = Option<double>(s); return *this; }
    FormatTagOptions &setAlign(AlignType a) { this->align = Option<AlignType>(a); return *this; }
    FormatTagOptions &setNewline() { this->newline = true; return *this; }
    FormatTagOptions &setCustom(CustomRenderFunction *c) { this->custom = c; return *this; }
    FormatTagOptions &setImage(std::shared_ptr<ImageData> image) { this->image = image; return *this; }
};

struct TextParser;

/**
 * Text using bitmap fonts, supporting format tags to change the size, color,
 * etc.
 */
struct BitmapText: public VisibleSprite {
    static std::unordered_map<std::string, FormatTagOptions> formatTags;
    static std::vector<std::shared_ptr<ImageData>> images;

    static void addFormatTag(std::string, FormatTagOptions);

    std::string text;
    Dimensions textDimensions;
    std::shared_ptr<BitmapFont> font;
    BitmapTextOptions options;
    int charCount = -1;
    int maxChars = 0;

    BitmapText(const BitmapTextOptions &options);

    BitmapText &setFont(shared_ptr<BitmapFont> font);
    BitmapText &setText(string text);
    BitmapText &setRichText(string text);
    BitmapText &refresh();

    double baseScale() { return static_cast<double>(this->options.size) / this->font->size; }
    Dimensions getSize() override {
        this->refresh();
        double s = this->baseScale();
        return Dimensions(
            this->textDimensions.width() * s,
            this->textDimensions.height() * s
        );
    }
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
