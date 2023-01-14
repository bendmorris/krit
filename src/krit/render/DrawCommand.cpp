#include "krit/render/DrawCommand.h"

#include <memory>

#include "krit/math/Matrix.h"
#include "krit/render/DrawKey.h"
#include "krit/render/ImageData.h"

namespace krit {
struct Triangle;

DrawCall &DrawCommandBuffer::getDrawCall(const DrawKey &key, int zIndex) {
    auto &types = this->buf.commandTypes;
    auto &drawCalls = buf.get<DrawTriangles>();
    int drawCallIndex = drawCalls.size() - 1;
    if (!types.empty() && types.back() == DrawTriangles) {
        bool fallingThrough = false;
        for (int typeIndex = types.size() - 1; typeIndex > -1; --typeIndex) {
            if (types[typeIndex] == DrawTriangles) {
                auto &drawCall = drawCalls[drawCallIndex];
                if (drawCall.matches(key)) {
                    return drawCall;
                }
                if (drawCall.zIndex <= zIndex) {
                    if (fallingThrough) {
                        // not a higher z index; we can't fall through anymore
                        auto &call = buf.emplace<DrawTriangles>(
                            typeIndex + 1, drawCallIndex + 1);
                        call.key = key;
                        call.zIndex = zIndex;
                        return call;
                    } else {
                        break;
                    }
                }
                --drawCallIndex;
                fallingThrough = true;
            } else {
                // we can't fall through anymore, as there's a non-DrawTriangles
                // command here
                auto &call = buf.emplace<DrawTriangles>(typeIndex + 1,
                                                        drawCallIndex + 1);
                call.key = key;
                call.zIndex = zIndex;
                return call;
            }
        }
    }
    auto &call = buf.emplace_back<DrawTriangles>();
    call.key = key;
    call.zIndex = zIndex;
    return call;
}

void DrawCommandBuffer::addTriangle(RenderContext &ctx, const DrawKey &key,
                                    const Triangle &t, const Triangle &uv,
                                    const Color &color, int zIndex) {
    if (color.a > 0 || key.shader) {
        DrawCall &call = this->getDrawCall(key, zIndex);
        addTriangle(call, t, uv, color, color, color);
    }
}

void DrawCommandBuffer::addTriangle(RenderContext &ctx, const DrawKey &key,
                                    const Triangle &t, const Triangle &uv,
                                    const Color &color1, const Color &color2,
                                    const Color &color3, int zIndex) {
    if (color1.a > 0 || color2.a > 0 || color3.a > 0 || key.shader) {
        DrawCall &call = this->getDrawCall(key, zIndex);
        addTriangle(call, t, uv, color1, color2, color3);
    }
}

void DrawCommandBuffer::addTriangle(DrawCall &draw, const Triangle &t,
                                    const Triangle &uv, const Color &color1,
                                    const Color &color2, const Color &color3) {
    size_t i = triangles.size();
    triangles.resize(i + 48);
    SpriteShader *s = draw.key.shader ? draw.key.shader
                                      : draw.key.image ? defaultTextureShader
                                                       : defaultColorShader;
    s->prepareVertex(triangles.data() + i, t.p1.x(), t.p1.y(), t.p1.z(),
                     uv.p1.x(), uv.p1.y(), color1);
    s->prepareVertex(triangles.data() + i + 16, t.p2.x(), t.p2.y(), t.p2.z(),
                     uv.p2.x(), uv.p2.y(), color2);
    s->prepareVertex(triangles.data() + i + 32, t.p3.x(), t.p3.y(), t.p3.z(),
                     uv.p3.x(), uv.p3.y(), color3);
    draw.indices.push_back(i / 16);
    draw.indices.push_back(i / 16 + 1);
    draw.indices.push_back(i / 16 + 2);

    if (boundsStack.size()) {
        float x1 = std::min({t.p1.x(), t.p2.x(), t.p3.x()});
        float x2 = std::max({t.p1.x(), t.p2.x(), t.p3.x()});
        float y1 = std::min({t.p1.y(), t.p2.y(), t.p3.y()});
        float y2 = std::max({t.p1.y(), t.p2.y(), t.p3.y()});
        float z1 = std::min({t.p1.z(), t.p2.z(), t.p3.z()});
        float z2 = std::max({t.p1.z(), t.p2.z(), t.p3.z()});
        for (auto &b : boundsStack) {
            updateBounds(b, x1, y1, x2, y2, z1, z2);
        }
    }
}

void DrawCommandBuffer::addTriangle(DrawCall &draw, float x1, float y1,
                                    float z1, float x2, float y2, float z2,
                                    float x3, float y3, float z3, float uv1,
                                    float uv2, float uv3, float uv4, float uv5,
                                    float uv6, const Color &color) {
    size_t i = triangles.size();
    triangles.resize(i + 48);
    SpriteShader *s = draw.key.shader ? draw.key.shader
                                      : draw.key.image ? defaultTextureShader
                                                       : defaultColorShader;
    s->prepareVertex(triangles.data() + i, x1, y1, z1, uv1, uv2, color);
    s->prepareVertex(triangles.data() + i + 16, x2, y2, z2, uv3, uv4, color);
    s->prepareVertex(triangles.data() + i + 32, x3, y3, z3, uv5, uv6, color);
    draw.indices.push_back(i / 16);
    draw.indices.push_back(i / 16 + 1);
    draw.indices.push_back(i / 16 + 2);

    if (boundsStack.size()) {
        float xmin = std::min({x1, x2, x3});
        float xmax = std::max({x1, x2, x3});
        float ymin = std::min({y1, y2, y3});
        float ymax = std::max({y1, y2, y3});
        float zmin = std::min({z1, z2, z3});
        float zmax = std::max({z1, z2, z3});
        for (auto &b : boundsStack) {
            // TODO: account for z here
            updateBounds(b, xmin, ymin, xmax, ymax, zmin, zmax);
        }
    }
}

void DrawCommandBuffer::addRect(RenderContext &ctx, const DrawKey &key,
                                const IntRectangle &rect, const Matrix4 &matrix,
                                const Color &color, int zIndex) {
    if (color.a <= 0 && !key.shader) {
        return;
    }

    DrawCall &draw = this->getDrawCall(key, zIndex);

    float uvx1;
    float uvy1;
    float uvx2;
    float uvy2;
    std::shared_ptr<ImageData> imageData = key.image;
    if (!imageData) {
        uvx1 = uvy1 = 0;
        uvx2 = rect.width;
        uvy2 = rect.height;
    } else {
        uvx1 = rect.x / static_cast<float>(imageData->width());
        uvy1 = rect.y / static_cast<float>(imageData->height());
        uvx2 = (rect.x + rect.width) / static_cast<float>(imageData->width());
        uvy2 = (rect.y + rect.height) / static_cast<float>(imageData->height());
    }

    // matrix transformations
    Vec4f ul(0, 0, 0, 1), ur(rect.width, 0, 0, 1), ll(0, rect.height, 0, 1),
        lr(rect.width, rect.height, 0, 1);
    ul = matrix * ul;
    ur = matrix * ur;
    ll = matrix * ll;
    lr = matrix * lr;

    size_t i = triangles.size();
    triangles.resize(i + 64);
    SpriteShader *s = draw.key.shader ? draw.key.shader
                                      : draw.key.image ? defaultTextureShader
                                                       : defaultColorShader;
    s->prepareVertex(triangles.data() + i, ul.x(), ul.y(), ul.z(), uvx1, uvy1,
                     color);
    s->prepareVertex(triangles.data() + i + 16, ur.x(), ur.y(), ur.z(), uvx2,
                     uvy1, color);
    s->prepareVertex(triangles.data() + i + 32, ll.x(), ll.y(), ll.z(), uvx1,
                     uvy2, color);
    s->prepareVertex(triangles.data() + i + 48, lr.x(), lr.y(), lr.z(), uvx2,
                     uvy2, color);
    draw.indices.push_back(i / 16);
    draw.indices.push_back(i / 16 + 1);
    draw.indices.push_back(i / 16 + 2);
    draw.indices.push_back(i / 16 + 2);
    draw.indices.push_back(i / 16 + 1);
    draw.indices.push_back(i / 16 + 3);

    if (boundsStack.size()) {
        float x1 = std::min({ll.x(), lr.x(), ul.x(), ur.x()});
        float x2 = std::max({ll.x(), lr.x(), ul.x(), ur.x()});
        float y1 = std::min({ll.y(), lr.y(), ul.y(), ur.y()});
        float y2 = std::max({ll.y(), lr.y(), ul.y(), ur.y()});
        float z1 = std::min({ll.z(), lr.z(), ul.z(), ur.z()});
        float z2 = std::max({ll.z(), lr.z(), ul.z(), ur.z()});
        for (auto &b : boundsStack) {
            updateBounds(b, x1, y1, x2, y2, z1, z2);
        }
    }
}

void DrawCommandBuffer::updateBounds(AutoClipBounds &bounds, float x1, float y1,
                                     float x2, float y2, float z1, float z2) {
    if (std::isnan(bounds.xRange.first) || bounds.xRange.first > x1) {
        bounds.xRange.first = x1;
    }
    if (std::isnan(bounds.xRange.second) || bounds.xRange.second < x2) {
        bounds.xRange.second = x2;
    }
    if (std::isnan(bounds.yRange.first) || bounds.yRange.first > y1) {
        bounds.yRange.first = y1;
    }
    if (std::isnan(bounds.yRange.second) || bounds.yRange.second < y2) {
        bounds.yRange.second = y2;
    }
    if (std::isnan(bounds.zRange.first) || bounds.zRange.first > z1) {
        bounds.zRange.first = z1;
    }
    if (std::isnan(bounds.zRange.second) || bounds.zRange.second < z2) {
        bounds.zRange.second = z2;
    }
}

void DrawCommandBuffer::startAutoClip(float xBuffer, float yBuffer) {
    buf.emplace_back<PushClipRect>();
    boundsStack.emplace_back();
    auto &clip = boundsStack.back();
    clip.clipIndex = buf.get<PushClipRect>().size() - 1;
    clip.xBuffer = xBuffer;
    clip.yBuffer = yBuffer;
}

bool DrawCommandBuffer::endAutoClip(RenderContext &ctx) {
    auto &clip = boundsStack.back();
    float x1 = NAN, x2 = NAN, y1 = NAN, y2 = NAN;
    int w = ctx.camera->viewportWidth(), h = ctx.camera->viewportHeight();
    Matrix4 m;
    m.identity();
    ctx.camera->getTransformationMatrix(m, w, h);
    for (int x = 0; x < 2; ++x) {
        float xi;
        if (x == 0) {
            xi = clip.xRange.first;
        } else if (clip.xRange.first == clip.xRange.second) {
            continue;
        } else {
            xi = clip.xRange.second;
        }
        for (int y = 0; y < 2; ++y) {
            float yi;
            if (y == 0) {
                yi = clip.yRange.first;
            } else if (clip.yRange.first == clip.yRange.second) {
                continue;
            } else {
                yi = clip.yRange.second;
            }
            for (int z = 0; z < 2; ++z) {
                float zi;
                if (z == 0) {
                    zi = clip.zRange.first;
                } else if (clip.zRange.first == clip.zRange.second) {
                    continue;
                } else {
                    zi = clip.zRange.second;
                }
                Vec4f v(xi, yi, zi, 1);
                v = m * v;
                float px = v.x() / v.w(), py = v.y() / v.w();
                if (std::isnan(x1) || px < x1) {
                    x1 = px;
                }
                if (std::isnan(x2) || px > x2) {
                    x2 = px;
                }
                if (std::isnan(y1) || py < y1) {
                    y1 = py;
                }
                if (std::isnan(y2) || py > y2) {
                    y2 = py;
                }
            }
        }
    }
    if (std::isnan(x1)) {
        x1 = x2 = y1 = y2 = 0;
    }
    x1 = ((x1 + 1) / 2) * w - clip.xBuffer;
    x2 = ((x2 + 1) / 2) * w + clip.xBuffer;
    y1 = ((-y1 + 1) / 2) * h + clip.yBuffer;
    y2 = ((-y2 + 1) / 2) * h - clip.yBuffer;
    Rectangle &r = buf.get<PushClipRect>()[clip.clipIndex];
    r.setTo(x1, y2, x2 - x1, y1 - y2);
    boundsStack.pop_back();

    return r.width > 0 && r.height > 0 && r.x <= w && r.right() >= 0 &&
           r.y <= h && r.bottom() >= 0;
}

}
