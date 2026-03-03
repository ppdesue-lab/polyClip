#ifndef SWEEPEVENT_H
#define SWEEPEVENT_H

#include "../Point.h"
#include "../geom/Segment.h"

namespace PolyOffset {

class SweepEvent {
public:
    Point p;
    bool isLeft;
    int polygonType;
    SweepEvent* otherSE;
    bool inOut;
    int edgeType;
    bool inside;

    SweepEvent(const Point& p, bool isLeft, int polyType, SweepEvent* otherSweepEvent = nullptr, int edgeType = 0)
        : p(p), isLeft(isLeft), polygonType(polyType), otherSE(otherSweepEvent), edgeType(edgeType), inside(false) {}

    Segment segment() {
        return Segment(p, otherSE->p);
    }

    bool isBelow(const Point& x) {
        if (isLeft) {
            return signedArea(p, otherSE->p, x) > 0;
        } else {
            return signedArea(otherSE->p, p, x) > 0;
        }
    }

    bool isAbove(const Point& x) {
        return !isBelow(x);
    }

private:
    double signedArea(const Point& p0, const Point& p1, const Point& p2) {
        return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
    }
};

} // namespace PolyOffset

#endif // SWEEPEVENT_H
