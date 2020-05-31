#ifndef KRIT_SPRITES_LAYOUT
#define KRIT_SPRITES_LAYOUT

#include "krit/asset/AssetContext.h"
#include "krit/Sprite.h"
#include <memory>
#include <stack>
#include <string>
#include <utility>

#define UI_GET(x, T) static_cast<T*>(this->layout.getById(x))
#define UI_GET_NODE(x) this->layout.getNodeById(x)
#define UI_DECL(x, T) T *x = static_cast<T*>(this->layout.getById(#x))
#define UI_DECL_NODE(x) LayoutNode *x##Node = this->layout.getNodeById(#x)

namespace krit {

enum PositionMode {
    PositionAbsolute,
    PositionHbox,
    PositionVbox,
};

// TODO: opportunity for pooling everything here

struct LayoutNode: public VisibleSprite {
    std::unique_ptr<VisibleSprite> sprite = nullptr;
    Measurement padding[4] = {Measurement(0), Measurement(0), Measurement(0), Measurement(0)};
    AnchoredMeasurement x = AnchoredMeasurement(Measurement(0), 0);
    AnchoredMeasurement y = AnchoredMeasurement(Measurement(0), 0);
    Measurement width = Measurement(Percent, 100);
    Measurement height = Measurement(Percent, 100);
    Measurement spacing = Measurement(0);
    bool stretch = false;
    bool keepSize = false;
    bool visible = true;

    PositionMode positionMode = PositionAbsolute;
    LayoutNode *parent = nullptr;
    LayoutNode *prevSibling = nullptr;
    double siblingSpacing = 0;

    LayoutNode() {}

    Measurement &paddingTop() { return this->padding[0]; }
    Measurement &paddingBottom() { return this->padding[1]; }
    Measurement &paddingLeft() { return this->padding[2]; }
    Measurement &paddingRight() { return this->padding[3]; }

    LayoutNode &setX(Measurement v, double anchor) {
        this->x.value = v;
        this->x.anchor = anchor;
        return *this;
    }

    LayoutNode &setY(Measurement v, double anchor) {
        this->y.value = v;
        this->y.anchor = anchor;
        return *this;
    }

    LayoutNode &setWidth(Measurement v) {
        this->width = v;
        return *this;
    }

    LayoutNode &setHeight(Measurement v) {
        this->height = v;
        return *this;
    }

    LayoutNode &setPadding(Measurement top, Measurement bottom, Measurement left, Measurement right) {
        this->paddingTop() = top;
        this->paddingBottom() = bottom;
        this->paddingLeft() = left;
        this->paddingRight() = right;
        return *this;
    }

    LayoutNode &setStretch(bool v) {
        this->stretch = v;
        return *this;
    }

    LayoutNode &attachSprite(VisibleSprite *e) {
        this->sprite = std::unique_ptr<VisibleSprite>(e);
        return *this;
    }

    bool isVisible() {
        return this->visible && (!this->parent || this->parent->isVisible());
    }

    void reflow(UpdateContext &ctx);

    Point getPosition() override { return this->position; }
    Dimensions getSize() override { return this->keepSize ? this->dimensions : (this->sprite ? this->sprite->getSize() : Dimensions(this->width.measure(0), this->height.measure(0))); }

    void update(UpdateContext &ctx) override;

    void render(RenderContext &ctx) override;
};

struct LayoutRoot;

struct DivData {
    LayoutNode *div;
    LayoutNode *lastChild = nullptr;
    PositionMode mode;

    DivData(LayoutNode *div, PositionMode mode): div(div), mode(mode) {}
};

struct LayoutParseData {
    std::string *path;
    LayoutRoot *root;
    LayoutNode *node;
    AssetContext *asset;
    std::stack<DivData> divs;
};

typedef void LayoutParseFunction(LayoutParseData *, std::unordered_map<std::string, std::string>&);

struct LayoutRoot: public Sprite {
    static std::unordered_map<std::string, LayoutParseFunction*> parsers;
    static void parseLayoutAttr(LayoutParseData *data, LayoutNode *layout, const std::string &key, const std::string &value);
    static SpriteStyle parseStyle(std::string &s);
    static void parseAndApplyStyle(std::unordered_map<std::string, std::string> &attrMap, VisibleSprite *e);
    static ImageRegion parseSrc(std::unordered_map<std::string, std::string> &attrMap, AssetContext *asset);

    static void addParser(const std::string &tag, LayoutParseFunction *f) {
        parsers.insert(std::make_pair(tag, f));
    }

    std::vector<std::unique_ptr<LayoutNode>> nodes;
    std::unordered_map<std::string, LayoutNode*> nodeMap;

    LayoutRoot(std::string path, AssetContext &asset);

    LayoutNode *getNodeById(const std::string &id) {
        auto found = this->nodeMap.find(id);
        return (found != this->nodeMap.end()) ? found->second : nullptr;
    }

    VisibleSprite *getById(const std::string &id) {
        LayoutNode *node = this->getNodeById(id);
        return node ? node->sprite.get() : nullptr;
    }

    void update(UpdateContext &ctx) override {
        for (std::unique_ptr<LayoutNode> &node : this->nodes) {
            node->update(ctx);
        }
    }

    void render(RenderContext &ctx) override {
        for (std::unique_ptr<LayoutNode> &node : this->nodes) {
            node->render(ctx);
        }
    }
};

}

#endif
