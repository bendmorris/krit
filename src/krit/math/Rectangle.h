#ifndef KRIT_MATH_RECTANGLE
#define KRIT_MATH_RECTANGLE

namespace krit {

template <typename T> class BaseRectangle {
    public:
        T x, y, width, height;

        BaseRectangle<T>() {}
        BaseRectangle<T>(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}

        bool operator==(const BaseRectangle<T> &other) {
            return this->x == other.x && this->y == other.y &&
                this->width == other.width && this->height == other.height;
        }

        T top() { return this->y; }
        T bottom() { return this->y + this->height; }
        T left() { return this->x; }
        T right() { return this->x + this->width; }

        bool eq(BaseRectangle<T> &other) {
            return
                this->x == other.x &&
                this->y == other.y &&
                this->width == other.width &&
                this->height == other.height;
        }

        bool overlaps(BaseRectangle<T> &other) {
            return this->right() >= other.left() &&
                this->bottom() >= other.top() &&
                this->left() <= other.right() &&
                this->top() <= other.bottom();
        }

        template <typename U> bool contains(U &other) {
            return (
                other.x >= left() &&
                other.x <= right() &&
                other.y >= top() &&
                other.y <= bottom()
            );
        }

        BaseRectangle<T> &setTo(BaseRectangle<T> &other) {
            return this->setTo(other.x, other.y, other.width, other.height);
        }

        BaseRectangle<T> &setTo(T x, T y, T w, T h) {
            this->x = x;
            this->y = y;
            this->width = w;
            this->height = h;
            return *this;
        }
};

class Rectangle: public BaseRectangle<double> {
    public:
        Rectangle() {}
        Rectangle(double x, double y, double width, double height) : BaseRectangle<double>(x, y, width, height) {}
        template<typename T, typename U> Rectangle(BasePoint<double, T> p, BasePoint<double, U> d) : Rectangle(p.x, p.y, d.x, d.y) {}

        bool contains(Point& p) {
            return p.x >= this->x && p.x <= (this->x + this->width) &&
                p.y >= this->y && p.y <= (this->y + this->height);
        }
};
class IntRectangle: public BaseRectangle<int> {
    public:
        IntRectangle() {}
        IntRectangle(int x, int y, int width, int height) : BaseRectangle<int>(x, y, width, height) {}
};

}

#endif
