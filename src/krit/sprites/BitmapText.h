#ifndef KRIT_SPRITES_BITMAPTEXT
#define KRIT_SPRITES_BITMAPTEXT

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>
#include <memory>

#include "krit/asset/BitmapFont.h"
#include "krit/render/BlendMode.h"
#include "krit/sprites/TextBase.h"
#include "krit/utils/Color.h"
#include "krit/utils/Option.h"
#include "krit/Sprite.h"
#include "krit/Math.h"
#include "krit/math/Dimensions.h"
#include "krit/math/ScaleFactor.h"

namespace krit {
struct RenderContext;

struct BitmapTextOptions {
    std::shared_ptr<BitmapFont> font;
    int size = 16;
    AlignType align = LeftAlign;
    bool wordWrap = false;
    float lineSpacing = 0;
    bool dynamic = true;

    BitmapTextOptions() {}
    BitmapTextOptions(std::shared_ptr<BitmapFont> font): font(font) {}

    BitmapTextOptions &setSize(int size) { this->size = size; return *this; }
    BitmapTextOptions &setAlign(AlignType align) { this->align = align; return *this; }
    BitmapTextOptions &setWordWrap(bool wrap) { this->wordWrap = wrap; return *this; }
    BitmapTextOptions &setLineSpacing(float spacing) { this->lineSpacing = spacing; return *this; }
};

struct BitmapText;

typedef void CustomRenderFunction(RenderContext*, BitmapText*, GlyphRenderData*);

enum BitmapTextOpcodeType {
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
    Tab,
};

union BitmapTextOpcodeData {
    bool present;
    Color color;
    float number;
    AlignType align;
    CustomRenderFunction *custom = nullptr;
    std::string_view text;
    NewlineData newLine;
    VisibleSprite *sprite;
    BitmapFont *font;

    BitmapTextOpcodeData(): present(false) {}
    BitmapTextOpcodeData(Color color): color(color) {}
    BitmapTextOpcodeData(float number): number(number) {}
    BitmapTextOpcodeData(AlignType align): align(align) {}
    BitmapTextOpcodeData(CustomRenderFunction *custom): custom(custom) {}
    BitmapTextOpcodeData(std::string_view text): text(text) {}
    BitmapTextOpcodeData(Dimensions d, AlignType a): newLine(d, a) {}
    BitmapTextOpcodeData(VisibleSprite *sprite): sprite(sprite) {}
    BitmapTextOpcodeData(BitmapFont *font): font(font) {}
};

struct BitmapTextOpcode {
    BitmapTextOpcodeType type;
    BitmapTextOpcodeData data;

    BitmapTextOpcode() = default;
    BitmapTextOpcode(BitmapTextOpcodeType type, BitmapTextOpcodeData data): type(type), data(data) {}
    BitmapTextOpcode(BitmapTextOpcodeType type): type(type) {}
};

struct FormatTagOptions {
    Option<Color> color;
    Option<float> scale;
    Option<AlignType> align;
    bool newline = false;
    bool tab = false;
    CustomRenderFunction *custom = nullptr;
    VisibleSprite *sprite = nullptr;
    BitmapFont *font = nullptr;

    FormatTagOptions() = default;

    FormatTagOptions &setColor(Color c) { this->color = Option<Color>(c); return *this; }
    FormatTagOptions &setScale(float s) { this->scale = Option<float>(s); return *this; }
    FormatTagOptions &setAlign(AlignType a) { this->align = Option<AlignType>(a); return *this; }
    FormatTagOptions &setNewline() { this->newline = true; return *this; }
    FormatTagOptions &setTab() { this->tab = true; return *this; }
    FormatTagOptions &setCustom(CustomRenderFunction *c) { this->custom = c; return *this; }
    FormatTagOptions &setSprite(VisibleSprite *s) { this->sprite = s; return *this; }
    FormatTagOptions &setFont(BitmapFont *f) { this->font = f; return *this; }
};

struct BitmapTextParser;

/**
 * Text using bitmap fonts, supporting format tags to change the size, color,
 * etc.
 */
struct BitmapText: public VisibleSprite {
    static std::unordered_map<std::string, FormatTagOptions> formatTags;

    static void addFormatTag(std::string, FormatTagOptions);

    std::string text;
    Dimensions textDimensions;
    BitmapTextOptions options;
    int charCount = -1;
    int maxChars = 0;
    std::vector<float> tabStops;
    Color baseColor = Color::white();

    BitmapText() = default;
    BitmapText(const BitmapTextOptions &options);

    BitmapText &setFont(std::shared_ptr<BitmapFont> font);
    BitmapText &setText(const std::string &text);
    BitmapText &setRichText(const std::string &text);
    BitmapText &refresh();

    float baseScale() { return static_cast<float>(this->options.size) / this->options.font->size; }
    Dimensions getSize() override {
        this->refresh();
        float s = this->baseScale();
        return Dimensions(
            this->textDimensions.width() * s * scale.x,
            this->textDimensions.height() * s * scale.y
        );
    }
    float width() { this->refresh(); float s = this->baseScale(); return this->textDimensions.width() * s * scale.x; }
    float height() { this->refresh(); float s = this->baseScale(); return this->textDimensions.height() * s * scale.y; }
    void resize(float, float) override;
    void render(RenderContext &ctx) override;

    private:
        std::vector<BitmapTextOpcode> opcodes;
        bool rich = false;
        bool dirty = false;
        friend struct BitmapTextParser;
};

}

#endif
