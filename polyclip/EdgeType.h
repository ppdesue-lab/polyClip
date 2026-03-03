#ifndef EDGETYPE_H
#define EDGETYPE_H

namespace PolyOffset {

class EdgeType {
public:
    static const int NORMAL = 0;
    static const int NON_CONTRIBUTING = 1;
    static const int SAME_TRANSITION = 2;
    static const int DIFFERENT_TRANSITION = 3;
};

} // namespace PolyOffset

#endif // EDGETYPE_H
