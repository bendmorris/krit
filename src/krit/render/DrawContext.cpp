#include "DrawContext.h"

namespace krit {

void DrawContext::line(const Vec2f &p1, const Vec2f &p2) {
    // create perpendicular delta vector
    Vec2f a(p2.x() - p1.x(), p2.y() - p1.y());
    a.normalize(this->lineThickness / 2);
    Vec2f b = a.perpendicular();
    this->drawQuad(p1.x() + b.x() - a.x(), p1.y() + b.y() - a.y(),
                   p1.x() - b.x() - a.x(), p1.y() - b.y() - a.y(),
                   p2.x() - b.x() + a.x(), p2.y() - b.y() + a.y(),
                   p2.x() + b.x() + a.x(), p2.y() + b.y() + a.y());
}

void DrawContext::polyline(const Vec2f *points, size_t pointsLength,
                           bool miterJoint) {
    if (pointsLength < 2) {
        return;
    }

    float halfThick = this->lineThickness / 2;
    Vec2f a;
    Vec2f b;
    Vec2f pos = points[0];
    Vec2f prev(pos.x() - points[1].x(), pos.y() - points[1].y());
    Vec2f inner;
    Vec2f outer;
    Vec2f nextPrev;

    a.copyFrom(pos);
    b.copyFrom(pos);

    // calculate first cap
    Vec2f next = prev.perpendicular();
    next.normalize(halfThick);
    a += next;
    b -= next;

    prev.normalize(1); // unit length

    bool over180 = false;
    float angle = 0;
    size_t index = 0;

    for (size_t i = 1; i < pointsLength; ++i) {
        index = i * 2;

        pos.copyFrom(points[index]);

        // vector v (difference between current and next)
        next.copyFrom(pos);
        next -= points[index + 1];
        next.normalize(1); // unit length
        nextPrev = next; // we clobber the "next" value so it needs to be saved
        over180 = prev.zcross(next) > 0;
        // calculate half angle from two vectors
        // normally this would require knowing the vector lengths but
        // because they both should be unit vectors we can ignore dividing
        // by length
        angle = acos(prev.dot(next)) / 2;

        inner.copyFrom(prev);
        inner += next;
        inner.perpendicular();
        if (over180) {
            inner.invert();
        }
        inner.normalize(halfThick / cos(angle));
        if (miterJoint) {
            outer.copyFrom(pos);
            outer -= inner;
        }
        inner += pos;

        // calculate joint points
        prev.perpendicular();
        prev.normalize(halfThick);

        next.perpendicular();
        next.normalize(halfThick);

        if (over180) {
            prev.invert();
            next.invert();
        }

        prev += pos;
        next += pos;

        // draw line connection
        if (over180) {
            this->drawTriangle(a, b, prev);
        } else {
            this->drawTriangle(a, b, inner);
        }
        this->drawTriangle(b, prev, inner);
        // draw bevel joint
        this->drawTriangle(next, prev, inner);
        if (miterJoint) {
            this->drawTriangle(next, prev, outer);
        }

        if (over180) {
            a = next;
            b = inner;
        } else {
            a = inner;
            b = next;
        }

        prev = nextPrev;
    }

    // end cap
    next.copyFrom(points[pointsLength - 1]);
    pos -= next;
    pos.perpendicular();
    pos.normalize(halfThick);
    prev.copyFrom(next);
    prev += pos;
    next -= pos;

    // draw final line
    this->drawTriangle(a, b, prev);
    this->drawTriangle(b, prev, next);
}

void DrawContext::rect(const Rectangle &r) {
    float t = this->lineThickness;
    float x = r.left();
    float y = r.top();
    float x2 = r.right();
    float y2 = r.bottom();
    // top
    this->line(Vec2f(x, y), Vec2f(x2, y));
    // bottom
    this->line(Vec2f(x, y2), Vec2f(x2, y2));
    // left
    this->line(Vec2f(x, y + t), Vec2f(x, y2 - t));
    // right
    this->line(Vec2f(x2, y + t), Vec2f(x2, y2 - t));
}

void DrawContext::rectFilled(const Rectangle &r) {
    this->drawQuad(r.left(), r.top(), r.right(), r.top(), r.right(), r.bottom(),
                   r.left(), r.bottom());
}

void DrawContext::circle(const Vec2f &center, float radius, size_t segments) {
    arc(center, radius, 0, M_PI * 2, segments);
}

void DrawContext::ring(const Vec2f &center, float radius, size_t segments) {
    arc(center, radius, 0, M_PI * 2, segments, 0);
}

void DrawContext::circleFilled(const Vec2f &center, float radius,
                               size_t segments) {
    float x = center.x();
    float y = center.y();
    float radians = (2 * M_PI) / segments;
    float x1 = x;
    float y1 = y + radius;
    auto c = this->color.withAlpha(this->alpha);
    for (size_t segment = 1; segment < segments + 1; ++segment) {
        float theta = segment * radians;
        float x2 = x + sin(theta) * radius;
        float y2 = y + cos(theta) * radius;
        this->addTriangle(x, y, x1, y1, x2, y2, c, c, c);
        x1 = x2;
        y1 = y2;
    }
}

void DrawContext::arc(const Vec2f &center, float radius, float startRads,
                      float angleRads, size_t segments, float innerAlpha) {
    float x = center.x();
    float y = center.y();
    float radians = angleRads / segments;
    float halfThick = this->lineThickness / 2;
    float innerRadius = radius - halfThick;
    float outerRadius = radius + halfThick;
    Vec2f inner;
    Vec2f outer;
    Vec2f lastOuter;
    Vec2f lastInner;
    Color co(this->color.withAlpha(this->alpha));
    Color ci(this->color.withAlpha(this->alpha * innerAlpha));

    for (size_t segment = 0; segment < segments + 1; ++segment) {
        float theta = startRads + segment * radians;
        float sinv = sin(theta);
        float cosv = cos(theta);
        inner.setTo(x + cosv * innerRadius, y - sinv * innerRadius);
        outer.setTo(x + cosv * outerRadius, y - sinv * outerRadius);

        if (segment != 0) {
            this->addTriangle(lastInner.x(), lastInner.y(), lastOuter.x(),
                              lastOuter.y(), outer.x(), outer.y(), ci, co, co);
            this->addTriangle(lastInner.x(), lastInner.y(), outer.x(),
                              outer.y(), inner.x(), inner.y(), ci, co, ci);
        }

        lastOuter = outer;
        lastInner = inner;
    }
}

void DrawContext::drawTriangle(const Vec2f &v1, const Vec2f &v2,
                               const Vec2f &v3) {
    auto c = this->color.withAlpha(this->alpha);
    addTriangle(v1.x(), v1.y(), v2.x(), v2.y(), v3.x(), v3.y(), c, c, c);
}

void DrawContext::drawQuad(float x1, float y1, float x2, float y2, float x3,
                           float y3, float x4, float y4) {
    auto c = this->color.withAlpha(this->alpha);
    this->addTriangle(x1, y1, x2, y2, x3, y3, c, c, c);
    this->addTriangle(x1, y1, x3, y3, x4, y4, c, c, c);
}

void DrawContext::addTriangle(float tx1, float ty1, float tx2, float ty2,
                              float tx3, float ty3, const Color &c1,
                              const Color &c2, const Color &c3) {
    DrawKey key;
    key.smooth = this->smooth;
    key.blend = this->blend;
    key.shader = this->shader;
    Triangle t(tx1, ty1, tx2, ty2, tx3, ty3);
    Triangle uv;
    this->context->addTriangle(key, t, uv, c1, c2, c3, this->zIndex);
}

}