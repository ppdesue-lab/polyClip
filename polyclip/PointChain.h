#ifndef POINTCHAIN_H
#define POINTCHAIN_H

#include "Point.h"
#include "geom/Segment.h"
#include <vector>

namespace PolyOffset {

class PointChain {
public:
    bool closed;
    std::vector<Point> pointList;

    PointChain(const Segment& s) : closed(false) {
        pointList.push_back(s.start);
        pointList.push_back(s.end);
    }

    bool linkSegment(const Segment& s) {
        Point front = pointList[0];
        Point back = pointList[pointList.size() - 1];

        if (s.start.equals(front)) {
            if (s.end.equals(back)) {
                closed = true;
            } else {
                pointList.insert(pointList.begin(), s.end);
            }
            return true;
        } else if (s.end.equals(back)) {
            if (s.start.equals(front)) {
                closed = true;
            } else {
                pointList.push_back(s.start);
            }
            return true;
        } else if (s.end.equals(front)) {
            if (s.start.equals(back)) {
                closed = true;
            } else {
                pointList.insert(pointList.begin(), s.start);
            }
            return true;
        } else if (s.start.equals(back)) {
            if (s.end.equals(front)) {
                closed = true;
            } else {
                pointList.push_back(s.end);
            }
            return true;
        }

        return false;
    }

    bool linkPointChain(PointChain* chain) {
        Point firstPoint = pointList[0];
        Point lastPoint = pointList[pointList.size() - 1];

        Point chainFront = chain->pointList[0];
        Point chainBack = chain->pointList[chain->pointList.size() - 1];

        if (chainFront.equals(lastPoint)) {
            pointList.pop_back();
            pointList.insert(pointList.end(), chain->pointList.begin(), chain->pointList.end());
            return true;
        }

        if (chainBack.equals(firstPoint)) {
            pointList.erase(pointList.begin());
            pointList.insert(pointList.begin(), chain->pointList.begin(), chain->pointList.end());
            return true;
        }

        if (chainFront.equals(firstPoint)) {
            pointList.erase(pointList.begin());
            std::reverse(chain->pointList.begin(), chain->pointList.end());
            pointList.insert(pointList.begin(), chain->pointList.begin(), chain->pointList.end());
            return true;
        }

        if (chainBack.equals(lastPoint)) {
            pointList.pop_back();
            std::reverse(pointList.begin(), pointList.end());
            pointList.insert(pointList.begin(), chain->pointList.begin(), chain->pointList.end());
            return true;
        }

        return false;
    }
};

} // namespace PolyOffset

#endif // POINTCHAIN_H
