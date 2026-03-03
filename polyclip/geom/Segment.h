#ifndef SEGMENT_H
#define SEGMENT_H

#include "../Point.h"

namespace PolyOffset {

class Segment {
public:
    Point start;
    Point end;

    Segment() : start(), end() {}
    Segment(const Point& start, const Point& end) : start(start), end(end) {}
};

} // namespace PolyOffset

#endif // SEGMENT_H
