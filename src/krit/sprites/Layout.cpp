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
    this->reflow(ctx);
    if (this->sprite) {
        this->sprite->update(ctx);
    }
}

void LayoutNode::render(RenderContext &ctx) {
    if (this->isVisible() && this->sprite) {
        this->sprite->render(ctx);
    }
}

void LayoutNode::reflow(UpdateContext &ctx) {
    double x, y, availableWidth, availableHeight;
    if (this->parent) {
        Point position = this->parent->position;
        Dimensions dims = this->parent->dimensions;
        double paddingTop = this->parent->paddingTop().measure(dims.height());
        double paddingBottom = this->parent->paddingBottom().measure(dims.height());
        double paddingLeft = this->parent->paddingLeft().measure(dims.width());
        double paddingRight = this->parent->paddingRight().measure(dims.width());
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
    double width = this->width.measure(availableWidth);
    double height = this->height.measure(availableHeight);

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
            if (!this->prevSibling) {
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
                Point siblingPos = this->prevSibling->getPosition();
                Dimensions siblingSize = this->prevSibling->getSize();
                if (horizontal) {
                    double spacing = this->parent->spacing.measure(availableWidth);
                    ex = siblingPos.x + siblingSize.width() + spacing;
                    ey = y + this->y.measure(availableHeight, spriteSize.height());
                } else {
                    double spacing = this->parent->spacing.measure(availableHeight);
                    ex = x + this->x.measure(availableWidth, spriteSize.width());
                    ey = siblingPos.y + siblingSize.height() + spacing;
                }
            }
            break;
        }
    }

    this->position.setTo(ex, ey);
    this->dimensions.setTo(width, height);
    if (this->sprite) {
        this->sprite->move(ex, ey);
        if (this->stretch) {
            this->sprite->resize(width, height);
        }
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

void parsePlaceholder(LayoutParseData *data, std::unordered_map<std::string, std::string> &attrMap) {
    data->node->keepSize = false;
}

void layoutStartElement(void *userData, const char *name, const char **attrs) {
    LayoutParseData *data = static_cast<LayoutParseData*>(userData);
    std::unordered_map<std::string, std::string> &attrMap = collectAttrs(attrs);
    LayoutNode *node = new LayoutNode();
    data->root->nodes.emplace_back(node);
    data->node = node;
    for (auto &it: attrMap) {
        const std::string &key = it.first;
        const std::string &value = it.second;
        LayoutRoot::parseLayoutAttr(data, node, key, value);
    }
    if (!data->divs.empty()) {
        auto &top = data->divs.top();
        node->parent = top.div;
        node->prevSibling = top.lastChild;
        node->positionMode = top.mode;
        top.lastChild = node;
    }
    if (!strcmp(name, "hbox") || !strcmp(name, "vbox")) {
        bool horizontal = name[0] == 'h';
        data->divs.push(DivData(node, horizontal ? PositionHbox : PositionVbox));
    } else {
        auto found = LayoutRoot::parsers.find(std::string(name));
        if (found != LayoutRoot::parsers.end()) {
            if (found->second) found->second(data, attrMap);
        }
        data->divs.push(DivData(node, PositionAbsolute));
    }
    std::unordered_map<std::string, std::string>::const_iterator it;
    if ((it = attrMap.find("id")) != attrMap.end()) {
        data->root->nodeMap.insert(std::make_pair(it->second, node));
    }
}

void layoutEndElement(void *userData, const char *name) {
    LayoutParseData *data = static_cast<LayoutParseData*>(userData);
    data->divs.pop();
}

std::unordered_map<std::string, LayoutParseFunction*> LayoutRoot::parsers = {
    {"div", &parseDiv},
    {"placeholder", &parsePlaceholder},
    {"img", &parseImg},
    {"backdrop", &parseBackdrop},
    {"label", &parseLabel},
    {"spine", &parseSpine},
    {"nineslice", &parseNineSlice},
    {"button", &parseButton},
};

LayoutRoot::LayoutRoot(std::string path, AssetContext &asset)
{
    XML_Parser parser = XML_ParserCreate(nullptr);
    LayoutParseData data;
    data.path = &path;
    data.node = nullptr;
    data.root = this;
    data.asset = &asset;
    XML_SetUserData(parser, &data);
    XML_SetElementHandler(parser, layoutStartElement, layoutEndElement);
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
}

}
