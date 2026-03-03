#ifndef POINT_H
#define POINT_H

#include <cmath>

namespace PolyOffset {

class Point {
public:
    double x, y;

    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}

    bool equals(const Point& other) const {
        return x == other.x && y == other.y;
    }

    double distance(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return sqrt(dx * dx + dy * dy);
    }

    double length() const {
        return sqrt(x * x + y * y);
    }
};

} // namespace PolyOffset

#endif // POINT_H
