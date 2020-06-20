// #ifndef KRIT_RENDER_DRAWCONTEXT
// #define KRIT_RENDER_DRAWCONTEXT

// #include "krit/render/BlendMode.h"
// #include "krit/render/RenderContext.h"
// #include "krit/utils/Color.h"
// #include "krit/Math.h"

// namespace krit {

// struct DrawContext {
//     RenderContext *render;
//     BlendMode blend;
//     bool smooth;
//     Color color;
//     float alpha;
//     double lineThickness = 1;

//     DrawContext &setColor(Color color, float alpha = 1) {
//         this->color = color;
//         this->alpha = alpha;
//         return *this;
//     }

//     void line(Point p1, Point p2) {
//         // create perpendicular delta vector
//         var a = struct Point {x: p2.x - p1.x, y: p2.y - p1.y};
//         a.normalize(this.lineThickness / 2);
//         var b = a.copy();
//         b.perpendicular();

//         this.drawQuad(
//             p1.x + b.x - a.x, p1.y + b.y - a.y,
//             p1.x - b.x - a.x, p1.y - b.y - a.y,
//             p2.x - b.x + a.x, p2.y - b.y + a.y,
//             p2.x + b.x + a.x, p2.y + b.y + a.y
//         );
//     }

//     public function polyline(points: Slice[Point], miterJoint: Bool) {
//         if points.length < 2 {
//             return;
//         }

//         var halfThick = this.lineThickness / 2;
//         var a        = P();
//         var b        = P();
//         var pos      = points[0];
//         var prev     = P(pos.x - points[1].x, pos.y - points[1].y); // direction
//         var next     = P(prev.x, prev.y);
//         var inner    = P();
//         var outer    = P();
//         var nextPrev = P();

//         a.copyFrom(pos);
//         b.copyFrom(pos);

//         // calculate first cap
//         next.perpendicular();
//         next.normalize(halfThick);
//         a.add(next);
//         b.subtract(next);

//         prev.normalize(1); // unit length

//         var over180: Bool = false;
//         var angle: Float = 0;
//         var index: Int = 0;

//         for i in 1 ... points.length {
//             index = i * 2;

//             pos.copyFrom(points[index]);

//             // vector v (difference between current and next)
//             next.copyFrom(pos).subtract(points[index + 1]);
//             next.normalize(1); // unit length
//             nextPrev.copyFrom(next); // we clobber the "next" value so it needs to be saved

//             over180 = prev.zcross(next) > 0;
//             // calculate half angle from two vectors
//             // normally this would require knowing the vector lengths but because
//             // they both should be unit vectors we can ignore dividing by length
//             angle = acos(prev.dot(next)) / 2;

//             inner.copyFrom(prev);
//             inner.add(next);
//             inner.perpendicular();
//             if over180 {
//                 inner.invert();
//             }
//             inner.normalize(halfThick / cos(angle));
//             if miterJoint {
//                 outer.copyFrom(pos);
//                 outer.subtract(inner);
//             }
//             inner.add(pos);

//             // calculate joint points
//             prev.perpendicular();
//             prev.normalize(halfThick);

//             next.perpendicular();
//             next.normalize(halfThick);

//             if !over180 {
//                 prev.invert();
//                 next.invert();
//             }

//             prev.add(pos);
//             next.add(pos);

//             // draw line connection
//             if over180 {
//                 this.drawTriangle(a, b, prev);
//             } else {
//                 this.drawTriangle(a, b, inner);
//             }
//             this.drawTriangle(b, prev, inner);
//             // draw bevel joint
//             this.drawTriangle(next, prev, inner);
//             if miterJoint {
//                 this.drawTriangle(next, prev, outer);
//             }

//             if over180 {
//                 a.copyFrom(next);
//                 b.copyFrom(inner);
//             } else {
//                 a.copyFrom(inner);
//                 b.copyFrom(next);
//             }

//             prev.copyFrom(nextPrev);
//         }

//         // end cap
//         next.copyFrom(points[points.length - 1]);
//         pos.subtract(next);
//         pos.perpendicular();
//         pos.normalize(halfThick);
//         prev.copyFrom(next);
//         prev.add(pos);
//         next.subtract(pos);

//         // draw final line
//         this.drawTriangle(a, b, prev);
//         this.drawTriangle(b, prev, next);
//     }

//     public function rect(r: Ptr[Rectangle[Float]]) {
//         var t = this.lineThickness;
//         var x = r.left;
//         var y = r.top;
//         var x2 = r.right;
//         var y2 = r.bottom;
//         this.line(struct Point {x: x, y}, struct Point {x: x2, y} ); // top
//         this.line(struct Point {x: x, y: y2}, struct Point {x: x2, y: y2}); // bottom
//         this.line(struct Point {x, y: y + t}, struct Point {x, y: y2 - t}); // left
//         this.line(struct Point {x: x2, y: y + t}, struct Point {x: x2, y: y2 - t}); // right
//     }

//     public function rectFilled(r: Ptr[Rectangle[Float]]) {
//         this.drawQuad(
//             r.x, r.y,
//             r.right, r.y,
//             r.right, r.bottom,
//             r.x, r.bottom
//         );
//     }

//     public function circle(center: Ptr[Point], radius: Float, segments: Int = 16) {
//         const x = center.x;
//         const y = center.y;
//         const radians = 2 * PI / segments;
//         const halfThick = this.lineThickness / 2;
//         const innerRadius = radius - halfThick;
//         const outerRadius = radius + halfThick;
//         var inner = struct Point;
//         var outer = struct Point;
//         var lastOuter = struct Point;
//         var lastInner = struct Point;

//         for segment in 0 ... segments + 1 {
//             const theta = segment * radians;
//             const sinv = sin(theta);
//             const cosv = cos(theta);
//             inner.setTo(x + sinv * innerRadius, y + cosv * innerRadius);
//             outer.setTo(x + sinv * outerRadius, y + cosv * outerRadius);

//             if segment != 0 {
//                 this.drawTriangle(lastInner, lastOuter, outer);
//                 this.drawTriangle(lastInner, outer, inner);
//             }

//             lastOuter.copyFrom(outer);
//             lastInner.copyFrom(inner);
//         }
//     }

//     public function circleFilled(center: Ptr[Point], radius: Float, segments: Int = 16) {
//         var x = center.x;
//         var y = center.y;
//         const radians = (2 * PI) / segments;
//         var x1 = x;
//         var y1 = y + radius;
//         for segment in 1 ... segments+1 {
//             var theta = segment * radians;
//             var x2 = x + sin(theta) * radius;
//             var y2 = y + cos(theta) * radius;
//             this.addTriangle(x, y, x1, y1, x2, y2, this.color.withAlpha(this.alpha));
//             x1 = x2;
//             y1 = y2;
//         }
//     }

//     var points: Vector[Point] = Vector.new(64);
//     public function arc(center: Point, radius: Float, startRads: Float, angleRads: Float, segments: Int = 16) {
//         var radians = angleRads / segments;
//         this.points.clear();
//         this.points.ensureSize(segments + 1);
//         for segment in 0 ... segments + 1 {
//             var theta = segment * radians + startRads;
//             this.points.push(P(center.x + cos(theta) * radius, center.y - sin(theta) * radius));
//         }
//         this.polyline(this.points.slice(), false);
//     }

//     // /**
//     //  * Draws a quadratic curve.
//     //  * @param    x1            X start.
//     //  * @param    y1            Y start.
//     //  * @param    x2            X control point, used to determine the curve.
//     //  * @param    y2            Y control point, used to determine the curve.
//     //  * @param    x3            X finish.
//     //  * @param    y3            Y finish.
//     //  * @param    segments    Increasing will smooth the curve but takes longer to render. Must be a value greater than zero.
//     //  */
//     // public function curve(x1:Int, y1:Int, x2:Int, y2:Int, x3:Int, y3:Int, segments:Int = 25)
//     // {
//     //     var points:Array<Float> = [];
//     //     points.push(x1);
//     //     points.push(y1);

//     //     var deltaT: Float = 1 / segments;

//     //     for (segment in 1...segments)
//     //     {
//     //         var t: Float = segment * deltaT;
//     //         var x: Float = (1 - t) * (1 - t) * x1 + 2 * t * (1 - t) * x2 + t * t * x3;
//     //         var y: Float = (1 - t) * (1 - t) * y1 + 2 * t * (1 - t) * y2 + t * t * y3;
//     //         points.push(x);
//     //         points.push(y);
//     //     }

//     //     points.push(x3);
//     //     points.push(y3);

//     //     polyline(points);
//     // }

//     function drawTriangle(v1: Point, v2: Point, v3: Point): Void {
//         this.addTriangle(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y, this.color.withAlpha(this.alpha));
//     }

//     function drawQuad(x1: Float, y1: Float, x2: Float, y2: Float, x3: Float, y3: Float, x4: Float, y4: Float) {
//         var c = this.color.withAlpha(this.alpha);
//         this.addTriangle(x1, y1, x2, y2, x3, y3, c);
//         this.addTriangle(x1, y1, x3, y3, x4, y4, c);
//     }

//     function addTriangle(
//         tx1: Float, ty1: Float,
//         tx2: Float, ty2: Float,
//         tx3: Float, ty3: Float,
//         color: ColorWithAlpha
//     ) {
//         var key = struct DrawKey {
//             smooth: this.smooth,
//             blend: this.blend,
//         };
//         this.context.addTriangle(
//             key,
//             struct Triangle {
//                 p1: struct Point {x: tx1, y: ty1},
//                 p2: struct Point {x: tx2, y: ty2},
//                 p3: struct Point {x: tx3, y: ty3},
//             },
//             struct Triangle,
//             color,
//             false
//         );
//     }
// };

// }

// #endif
