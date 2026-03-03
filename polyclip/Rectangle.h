#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Point.h"
#include <algorithm>

namespace PolyOffset {

class Rectangle {
public:
    double x, y, width, height;

    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(double x, double y, double width, double height) 
        : x(x), y(y), width(width), height(height) {}

    double getRight() const {
        return x + width;
    }

    double getBottom() const {
        return y + height;
    }

    bool intersects(const Rectangle& other) const {
        return !(getRight() < other.x || x > other.getRight() || 
                 getBottom() < other.y || y > other.getBottom());
    }

    Rectangle unionRect(const Rectangle& other) const {
        double minX = std::min(x, other.x);
        double minY = std::min(y, other.y);
        double maxX = std::max(getRight(), other.getRight());
        double maxY = std::max(getBottom(), other.getBottom());
        return Rectangle(minX, minY, maxX - minX, maxY - minY);
    }
};

} // namespace PolyOffset

#endif // RECTANGLE_H
