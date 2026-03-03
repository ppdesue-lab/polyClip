#ifndef CONTOUR_H
#define CONTOUR_H

#include "Segment.h"
#include "../Rectangle.h"
#include <vector>

namespace PolyOffset {

class Contour {
public:
    std::vector<Point> points;
    Rectangle* bounds;

    Contour() : bounds(nullptr) {}

    void add(const Point& p) {
        points.push_back(p);
    }

    Rectangle boundingBox() {
        if (bounds) {
            return *bounds;
        }

        double minX = INFINITY, minY = INFINITY;
        double maxX = -INFINITY, maxY = -INFINITY;

        for (const Point& p : points) {
            if (p.x > maxX) maxX = p.x;
            if (p.x < minX) minX = p.x;
            if (p.y > maxY) maxY = p.y;
            if (p.y < minY) minY = p.y;
        }

        bounds = new Rectangle(minX, minY, maxX - minX, maxY - minY);
        return *bounds;
    }

    Segment getSegment(int index) {
        if (index == points.size() - 1) {
            return Segment(points[points.size() - 1], points[0]);
        }
        return Segment(points[index], points[index + 1]);
    }

    bool containsPoint(const Point& p) {
        int intersections = 0;
        for (int i = 0; i < points.size(); i++) {
            const Point& curr = points[i];
            const Point& next = (i == points.size() - 1) ? points[0] : points[i + 1];

            if ((p.y < next.y && p.y > curr.y) || (p.y < curr.y && p.y > next.y)) {
                if (p.x < std::max(curr.x, next.x) && next.y != curr.y) {
                    double xInt = (p.y - curr.y) * (next.x - curr.x) / (next.y - curr.y) + curr.x;
                    if (curr.x == next.x || p.x <= xInt) {
                        intersections++;
                    }
                }
            }
        }

        return intersections % 2 != 0;
    }

    ~Contour() {
        if (bounds) {
            delete bounds;
        }
    }
};

} // namespace PolyOffset

#endif // CONTOUR_H
