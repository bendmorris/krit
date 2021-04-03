#ifndef KRIT_SPRITES_LAYOUT
#define KRIT_SPRITES_LAYOUT

#include "krit/asset/AssetContext.h"
#include "krit/Sprite.h"
#include <memory>
#include <stack>
#include <string>
#include <utility>

#define UI_GET_FROM(from, x, T) static_cast<T*>(from.layout.getById(#x))
#define UI_GET(x, T) UI_GET_FROM((*this), x, T)
#define UI_GET_NODE_FROM(from, x) from.layout.getNodeById(#x)
#define UI_GET_NODE(x) UI_GET_NODE_FROM((*this), x)
#define UI_DECL_FROM(from, x, T) T *x = static_cast<T*>(from.layout.getById(#x))
#define UI_DECL(x, T) UI_DECL_FROM((*this), x, T)
#define UI_DECL_NODE_FROM(from, x) LayoutNode *x##Node = from.layout.getNodeById(#x)
#define UI_DECL_NODE(x) UI_DECL_NODE_FROM((*this), x)

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
    bool clip = false;

    PositionMode positionMode = PositionAbsolute;
    PositionMode childPositionMode = PositionAbsolute;
    std::unique_ptr<LayoutNode> nextSibling;
    std::unique_ptr<LayoutNode> firstChild;
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

    VisibleSprite *getSprite() { return this->sprite.get(); }

    void reflow(UpdateContext &ctx, LayoutNode *parent, LayoutNode *prevSibling);

    Point getPosition() override { return this->position; }
    Dimensions getSize() override { return this->keepSize ? this->dimensions : (this->sprite ? this->sprite->getSize() : Dimensions(this->width.measure(0), this->height.measure(0))); }

    void update(UpdateContext &ctx) override;

    void render(RenderContext &ctx) override;
};

struct LayoutRoot;

struct LayoutParseData {
    std::string *path;
    LayoutRoot *root;
    LayoutNode *node;
    AssetContext *asset;
    std::stack<LayoutNode*> layoutStack;
};

typedef void LayoutParseFunction(LayoutParseData *, std::unordered_map<std::string, std::string>&);

struct LayoutRoot: public Sprite {
    static LayoutRoot *fromMarkup(const std::string &markup, AssetContext &asset) {
        LayoutRoot *root = new LayoutRoot();
        root->parse(markup, asset);
        return root;
    }

    static std::unordered_map<std::string, LayoutParseFunction*> parsers;
    static void parseLayoutAttr(LayoutParseData *data, LayoutNode *layout, const std::string &key, const std::string &value);
    static SpriteStyle parseStyle(std::string &s);
    static void parseAndApplyStyle(std::unordered_map<std::string, std::string> &attrMap, VisibleSprite *e);
    static ImageRegion parseSrc(std::unordered_map<std::string, std::string> &attrMap, AssetContext *asset);

    static void addParser(const std::string &tag, LayoutParseFunction *f) {
        parsers.insert(std::make_pair(tag, f));
    }

    std::unique_ptr<LayoutNode> rootNode;
    std::unordered_map<std::string, LayoutNode*> nodeMap;

    LayoutRoot() {}
    LayoutRoot(const std::string &path, AssetContext &asset);

    void parse(const char *markup, size_t len, AssetContext &asset);
    void parse(const std::string &markup, AssetContext &asset);

    LayoutNode *getNodeById(const std::string &id) {
        auto found = this->nodeMap.find(id);
        return (found != this->nodeMap.end()) ? found->second : nullptr;
    }

    VisibleSprite *getById(const std::string &id) {
        LayoutNode *node = this->getNodeById(id);
        return node ? node->sprite.get() : nullptr;
    }

    void update(UpdateContext &ctx) override {
        if (rootNode) {
            rootNode->update(ctx);
            rootNode->reflow(ctx, nullptr, nullptr);
        }
    }

    void render(RenderContext &ctx) override {
        if (rootNode) {
            rootNode->render(ctx);
        }
    }
};

}

#endif
