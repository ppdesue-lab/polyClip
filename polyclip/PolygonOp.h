#ifndef POLYGONOP_H
#define POLYGONOP_H

namespace PolyOffset {

class PolygonOp {
public:
    static const int UNION = 0;
    static const int INTERSECTION = 1;
    static const int DIFFERENCE = 2;
};

} // namespace PolyOffset

#endif // POLYGONOP_H
