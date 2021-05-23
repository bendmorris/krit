#include "krit/Engine.h"
#include "krit/sprites/Backdrop.h"
#include "krit/sprites/BitmapText.h"
#include "krit/sprites/Button.h"
#include "krit/sprites/Image.h"
#include "krit/sprites/Layout.h"
#include "krit/sprites/NineSlice.h"
#include "krit/sprites/SpineSprite.h"
#include "krit/utils/Panic.h"
#include "krit/utils/Parse.h"
#include "expat.h"
#include <sstream>
#include <string>

namespace krit {

void LayoutNode::update(UpdateContext &ctx) {
    if (this->sprite) {
        this->sprite->update(ctx);
    }
    LayoutNode *child = this->firstChild.get();
    while (child) {
        child->update(ctx);
        child = child->nextSibling.get();
    }
}

void LayoutNode::render(RenderContext &ctx) {
    if (!this->visible) {
        return;
    }
    if (this->clip) {
        ctx.pushClip(Rectangle(this->position.x, this->position.y, this->dimensions.width(), this->dimensions.height()));
    }
    if (this->sprite) {
        this->sprite->render(ctx);
    }
    LayoutNode *child = this->firstChild.get();
    while (child) {
        child->render(ctx);
        child = child->nextSibling.get();
    }
    if (this->clip) {
        ctx.popClip();
    }
}

void LayoutNode::measure(UpdateContext &ctx, LayoutNode *parent, LayoutNode *prevSibling) {
    double availableWidth, availableHeight;
    if (parent) {
        Dimensions dims = parent->dimensions;
        double paddingTop = parent->paddingTop().measure(dims.height());
        double paddingBottom = parent->paddingBottom().measure(dims.height());
        double paddingLeft = parent->paddingLeft().measure(dims.width());
        double paddingRight = parent->paddingRight().measure(dims.width());
        availableWidth = dims.width() - paddingLeft - paddingRight;
        availableHeight = dims.height() - paddingTop - paddingBottom;
    } else {
        availableWidth = ctx.window->width() / ctx.camera->scale.x;
        availableHeight = ctx.window->height() / ctx.camera->scale.y;
    }
    double width = this->width.measure(availableWidth);
    double height = this->height.measure(availableHeight);

    this->dimensions.setTo(width, height);
    if (this->sprite && this->stretch) {
        this->sprite->resize(width, height);
    }

    LayoutNode *child = this->firstChild.get(), *prevChild = nullptr;
    while (child) {
        child->measure(ctx, this, prevChild);
        prevChild = child;
        child = child->nextSibling.get();
    }
}

void LayoutNode::arrange(UpdateContext &ctx, LayoutNode *parent, LayoutNode *prevSibling) {
    double x, y, availableWidth, availableHeight;
    if (parent) {
        Point position = parent->position;
        Dimensions dims = parent->dimensions;
        double paddingTop = parent->paddingTop().measure(dims.height());
        double paddingBottom = parent->paddingBottom().measure(dims.height());
        double paddingLeft = parent->paddingLeft().measure(dims.width());
        double paddingRight = parent->paddingRight().measure(dims.width());
        x = position.x + paddingLeft;
        y = position.y + paddingTop;
        availableWidth = dims.width() - paddingLeft - paddingRight;
        availableHeight = dims.height() - paddingTop - paddingBottom;
    } else {
        x = -ctx.camera->offset.x;
        y = -ctx.camera->offset.y;
        availableWidth = ctx.window->width() / ctx.camera->scale.x;
        availableHeight = ctx.window->height() / ctx.camera->scale.y;
    }
    Dimensions spriteSize = this->getSize();

    double ex, ey;
    switch (this->positionMode) {
        case PositionAbsolute: {
            // absolute layout
            ex = x + this->x.measure(availableWidth, spriteSize.width());
            ey = y + this->y.measure(availableHeight, spriteSize.height());
            break;
        }
        case PositionHbox:
        case PositionVbox: {
            bool horizontal = this->positionMode == PositionHbox;
            if (!prevSibling) {
                // first child element; set to top left
                if (horizontal) {
                    ex = x;
                    ey = y + this->y.measure(availableHeight, spriteSize.height());
                } else {
                    ex = x + this->x.measure(availableWidth, spriteSize.width());
                    ey = y;
                }
            } else {
                // position relative to previous child
                Point siblingPos = prevSibling->getPosition();
                Dimensions siblingSize = prevSibling->getSize();
                if (horizontal) {
                    double spacing = parent->spacing.measure(availableWidth);
                    ex = siblingPos.x + siblingSize.width() + spacing;
                    ey = y + this->y.measure(availableHeight, spriteSize.height());
                } else {
                    double spacing = parent->spacing.measure(availableHeight);
                    ex = x + this->x.measure(availableWidth, spriteSize.width());
                    ey = siblingPos.y + siblingSize.height() + spacing;
                }
            }
            break;
        }
    }

    this->position.setTo(ex, ey);
    if (this->sprite) {
        this->sprite->move(ex, ey);
    }

    LayoutNode *child = this->firstChild.get(), *prevChild = nullptr;
    while (child) {
        child->arrange(ctx, this, prevChild);
        prevChild = child;
        child = child->nextSibling.get();
    }
}

std::unordered_map<std::string, std::string> &collectAttrs(const char **attrs) {
    static std::unordered_map<std::string, std::string> _attrMap;
    _attrMap.clear();
    const char **attr = attrs;
    while (*attr) {
        if (*(attr + 1)) {
            const char *key = *attr;
            const char *value = *(attr + 1);
            _attrMap.insert(std::make_pair(std::string(key), std::string(value)));
        }
        attr = attr + 2;
    }
    return _attrMap;
}

void LayoutRoot::parseLayoutAttr(LayoutParseData *data, LayoutNode *layout, const std::string &key, const std::string &value) {
    if (key == "x" || key == "left") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setX(val, 0);
    } else if (key == "right") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setX(val, 1);
    } else if (key == "centerX") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setX(val, 0.5);
    } else if (key == "y" || key == "top") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setY(val, 0);
    } else if (key == "bottom") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setY(val, 1);
    } else if (key == "centerY") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setY(val, 0.5);
    } else if (key == "width") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setWidth(val);
    } else if (key == "height") {
        Measurement val = ParseUtil::parseMeasurement(value);
        layout->setHeight(val);
    } else if (key == "visible") {
        layout->visible = ParseUtil::parseBool(value);
    } else if (key == "spacing") {
        layout->spacing = ParseUtil::parseMeasurement(value);
    } else if (key == "stretch") {
        layout->stretch = ParseUtil::parseBool(value);
    } else if (key == "padding") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[0] = paddingVal;
        layout->padding[1] = paddingVal;
        layout->padding[2] = paddingVal;
        layout->padding[3] = paddingVal;
    } else if (key == "paddingY") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[0] = paddingVal;
        layout->padding[1] = paddingVal;
    } else if (key == "paddingX") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[2] = paddingVal;
        layout->padding[3] = paddingVal;
    } else if (key == "paddingTop") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[0] = paddingVal;
    } else if (key == "paddingBottom") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[1] = paddingVal;
    } else if (key == "paddingLeft") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[2] = paddingVal;
    } else if (key == "paddingRight") {
        Measurement paddingVal = ParseUtil::parseMeasurement(value);
        layout->padding[3] = paddingVal;
    }
}

SpriteStyle LayoutRoot::parseStyle(std::string &s) {
    SpriteStyle style;
    std::stringstream stream(s);
    std::string token, key, value;
    while (getline(stream, token, ';')) {
        size_t index = token.find(":");
        if (index != std::string::npos) {
            key = token.substr(0, index);
            value = token.substr(index + 1);
            if (key == "color") {
                style.color = ParseUtil::parseColor(value);
            } else if (key == "alpha") {
                style.color.a = ParseUtil::parseFloat(value);
            } else if (key == "scaleX") {
                style.scale.x = ParseUtil::parseFloat(value);
            } else if (key == "scaleY") {
                style.scale.y = ParseUtil::parseFloat(value);
            }
        }
    }
    return style;
}

void LayoutRoot::parseAndApplyStyle(std::unordered_map<std::string, std::string> &attrMap, VisibleSprite *e) {
    auto found = attrMap.find("style");
    if (found != attrMap.end()) {
        e->applyStyle(LayoutRoot::parseStyle(found->second));
    }
}

ImageRegion LayoutRoot::parseSrc(std::unordered_map<std::string, std::string> &attrMap, AssetContext *asset) {
    auto found = attrMap.find("src");
    if (found != attrMap.end()) {
        return ImageRegion(asset->getImage(found->second));
    }
    found = attrMap.find("atlas");
    if (found != attrMap.end()) {
        std::shared_ptr<TextureAtlas> atlas = asset->getTextureAtlas(found->second);
        return atlas->getRegion(attrMap["region"]);
    }
    panic("couldn't find src");
}

void parseBackdrop(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    LayoutNode *node = data->node;
    ImageRegion src = LayoutRoot::parseSrc(attrMap, data->asset);
    Backdrop *backdrop = new Backdrop(src);
    node->attachSprite(backdrop);
    for (auto &it: attrMap) {
        const std::string &key = it.first;
        const std::string &value = it.second;
        if (key == "color") {
            backdrop->color = ParseUtil::parseColor(value);
        } else if (key == "alpha") {
            backdrop->color.a = ParseUtil::parseFloat(value);
        }
    }
}

void parseImg(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    LayoutNode *node = data->node;
    ImageRegion src = LayoutRoot::parseSrc(attrMap, data->asset);
    Image *img = new Image(src);
    LayoutRoot::parseAndApplyStyle(attrMap, img);
    node->attachSprite(img);
}

void parseLabel(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    LayoutNode *node = data->node;

    // parse text options first
    std::shared_ptr<BitmapFont> font = data->asset->getBitmapFont(attrMap["font"]);
    BitmapTextOptions options(font);
    std::unordered_map<std::string, std::string>::const_iterator it;
    if ((it = attrMap.find("size")) != attrMap.end()) {
        options.size = stoi(it->second);
    }
    if ((it = attrMap.find("wrap")) != attrMap.end()) {
        options.wordWrap = ParseUtil::parseBool(it->second);
    }
    BitmapText *txt = new BitmapText(options);
    LayoutRoot::parseAndApplyStyle(attrMap, txt);
    node->attachSprite(txt);

    // parse text or rich text
    if ((it = attrMap.find("rich")) != attrMap.end()) {
        txt->setRichText(it->second);
    } else if ((it = attrMap.find("text")) != attrMap.end()) {
        txt->setText(it->second);
    }

    // everything else
    for (auto &it: attrMap) {
        const std::string &key = it.first;
        const std::string &value = it.second;
        if (key == "align") {
            if (value == "left") txt->options.align = LeftAlign;
            else if (value == "center") txt->options.align = CenterAlign;
            else if (value == "right") txt->options.align = RightAlign;
            else panic("unexpected value for text alignment: %s", value.c_str());
        } else if (key == "tabStops") {
            std::stringstream stream(value);
            std::string token;
            while (getline(stream, token, ',')) {
                int stop = atoi(token.c_str());
                txt->tabStops.push_back(stop);
            }
        } else if (key == "baseColor") {
            txt->baseColor = ParseUtil::parseColor(value);
        }
    }
}

void parseSpine(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    LayoutNode *node = data->node;

    // parse skeleton name
    SpineSprite *spine = new SpineSprite(*data->asset, attrMap["skeleton"]);
    LayoutRoot::parseAndApplyStyle(attrMap, spine);
    spine->setDefaultMix(2.5/60);
    node->attachSprite(spine);

    // everything else
    for (auto &it: attrMap) {
        const std::string &key = it.first;
        const std::string &value = it.second;
        if (key == "setupAnimation") {
            spine->setAnimation(0, value, false);
        } else if (key == "animation") {
            spine->setAnimation(1, value, true);
        } else if (key == "skin") {
            spine->setSkin(value);
        } else if (key == "skin2") {
            spine->setSkin(value);
        } else if (key == "flipX") {
            spine->scale.x = ParseUtil::parseBool(value) ? -1 : 1;
        }
    }
}

void parseNineSlice(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    LayoutNode *node = data->node;
    ImageRegion src = LayoutRoot::parseSrc(attrMap, data->asset);
    int lw, rw, th, bh;
    auto found = attrMap.find("border");
    if (found != attrMap.end()) {
        lw = rw = th = bh = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("borderWidth");
    if (found != attrMap.end()) {
        lw = rw = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("borderHeight");
    if (found != attrMap.end()) {
        th = bh = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("lw");
    if (found != attrMap.end()) {
        lw = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("rw");
    if (found != attrMap.end()) {
        rw = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("th");
    if (found != attrMap.end()) {
        th = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("bh");
    if (found != attrMap.end()) {
        bh = ParseUtil::parseInt(found->second);
    }
    NineSlice *img = new NineSlice(src, lw, rw, th, bh);
    LayoutRoot::parseAndApplyStyle(attrMap, img);
    node->attachSprite(img);
}

void parseButton(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    LayoutNode *node = data->node;
    ImageRegion src = LayoutRoot::parseSrc(attrMap, data->asset);
    int lw, rw, th, bh;
    auto found = attrMap.find("border");
    if (found != attrMap.end()) {
        lw = rw = th = bh = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("borderWidth");
    if (found != attrMap.end()) {
        lw = rw = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("borderHeight");
    if (found != attrMap.end()) {
        th = bh = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("lw");
    if (found != attrMap.end()) {
        lw = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("rw");
    if (found != attrMap.end()) {
        rw = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("th");
    if (found != attrMap.end()) {
        th = ParseUtil::parseInt(found->second);
    }
    found = attrMap.find("bh");
    if (found != attrMap.end()) {
        bh = ParseUtil::parseInt(found->second);
    }
    std::shared_ptr<BitmapFont> font = data->asset->getBitmapFont(attrMap["font"]);
    BitmapTextOptions options(font);
    std::unordered_map<std::string, std::string>::const_iterator it;
    if ((it = attrMap.find("size")) != attrMap.end()) {
        options.size = stoi(it->second);
    }
    std::string &label = attrMap["label"];
    Button *btn = new Button(src, lw, rw, th, bh, options, label);

    found = attrMap.find("style");
    if (found != attrMap.end()) {
        btn->defaultStyle = LayoutRoot::parseStyle(found->second);
        btn->applyStyle(btn->defaultStyle);
    }
    found = attrMap.find("focused");
    if (found != attrMap.end()) {
        btn->focusedStyle = LayoutRoot::parseStyle(found->second);
    }
    found = attrMap.find("pressed");
    if (found != attrMap.end()) {
        btn->pressedStyle = LayoutRoot::parseStyle(found->second);
    }
    found = attrMap.find("labelStyle");
    if (found != attrMap.end()) {
        btn->label.applyStyle(LayoutRoot::parseStyle(found->second));
    }

    node->attachSprite(btn);
}

void parseDiv(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    data->node->keepSize = true;
}

void parseClip(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    data->node->keepSize = true;
    data->node->clip = true;
}

void parsePlaceholder(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    data->node->keepSize = false;
}

void layoutStartElement(void *userData, const char *name, const char **attrs) {
    LayoutParseData *data = static_cast<LayoutParseData*>(userData);
    std::unordered_map<std::string, std::string> &attrMap = collectAttrs(attrs);
    LayoutNode *node = new LayoutNode();
    data->node = node;
    for (auto &it: attrMap) {
        const std::string &key = it.first;
        const std::string &value = it.second;
        LayoutRoot::parseLayoutAttr(data, node, key, value);
    }
    if (!data->layoutStack.empty()) {
        auto top = data->layoutStack.top();
        node->positionMode = top->childPositionMode;
    }
    if (!strcmp(name, "hbox") || !strcmp(name, "vbox")) {
        bool horizontal = name[0] == 'h';
        node->childPositionMode = horizontal ? PositionHbox : PositionVbox;
    } else {
        auto found = LayoutRoot::parsers.find(std::string(name));
        if (found != LayoutRoot::parsers.end()) {
            if (found->second) found->second(data, attrMap);
        }
    }
    data->layoutStack.push(node);
    std::unordered_map<std::string, std::string>::const_iterator it;
    if ((it = attrMap.find("id")) != attrMap.end()) {
        data->root->nodeMap.insert(std::make_pair(it->second, node));
    }
}

void layoutEndElement(void *userData, const char *name) {
    LayoutParseData *data = static_cast<LayoutParseData*>(userData);
    auto node = data->layoutStack.top();
    data->layoutStack.pop();
    if (data->layoutStack.empty()) {
        // this is the layout root
        data->node = node;
    } else {
        // parent this node
        auto parent = data->layoutStack.top();
        parent->addChild(node);
    }
}

std::unordered_map<std::string, LayoutParseFunction*> LayoutRoot::parsers = {
    {"div", &parseDiv},
    {"clip", &parseClip},
    {"placeholder", &parsePlaceholder},
    {"img", &parseImg},
    {"backdrop", &parseBackdrop},
    {"label", &parseLabel},
    {"spine", &parseSpine},
    {"nineslice", &parseNineSlice},
    {"button", &parseButton},
};

LayoutRoot::LayoutRoot(const std::string &path, AssetContext &asset) {
    XML_Parser parser = XML_ParserCreate(nullptr);
    LayoutParseData data;
    data.path = (std::string*)&path;
    data.node = nullptr;
    data.root = this;
    data.asset = &asset;
    XML_SetUserData(parser, &data);
    XML_SetElementHandler(parser, layoutStartElement, layoutEndElement);
    // FIXME: use the regular asset system for this
    char buf[1024];
    FILE *fp = fopen(path.c_str(), "rb");
    do {
        int readLength = fread(buf, 1, 1024, fp);
        if (!readLength) {
            break;
        }
        if (XML_Parse(parser, buf, readLength, readLength < 1024 ? 1 : 0) == XML_STATUS_ERROR) {
            panic("%s: failed to parse layout XML!", path.c_str());
        }
    } while (true);
    XML_ParserFree(parser);
    this->rootNode = std::unique_ptr<LayoutNode>(data.node);
}

void LayoutRoot::parse(const std::string &markup, AssetContext &asset) {
    this->parse(markup.c_str(), markup.size(), asset);
}

void LayoutRoot::parse(const char *markup, size_t length, AssetContext &asset) {
    XML_Parser parser = XML_ParserCreate(nullptr);
    LayoutParseData data;
    data.path = nullptr;
    data.node = nullptr;
    data.root = this;
    data.asset = &asset;
    XML_SetUserData(parser, &data);
    XML_SetElementHandler(parser, layoutStartElement, layoutEndElement);
    if (XML_Parse(parser, markup, length, 1) == XML_STATUS_ERROR) {
        panic("failed to parse layout XML!\n\n%s\n", markup);
    }
    XML_ParserFree(parser);
    this->rootNode = std::unique_ptr<LayoutNode>(data.node);
}

}
