#ifndef SWEEPEVENTSET_H
#define SWEEPEVENTSET_H

#include "SweepEvent.h"
#include <vector>
#include <algorithm>

namespace PolyOffset {

class SweepEventSet {
public:
    std::vector<SweepEvent*> eventSet;

    SweepEventSet() {}

    void remove(SweepEvent* key) {
        auto it = std::find(eventSet.begin(), eventSet.end(), key);
        if (it != eventSet.end()) {
            eventSet.erase(it);
        }
    }

    int insert(SweepEvent* item) {
        int length = eventSet.size();
        if (length == 0) {
            eventSet.push_back(item);
            return 0;
        }

        eventSet.push_back(nullptr);

        int i = length - 1;
        while (i >= 0 && segmentCompare(item, eventSet[i])) {
            eventSet[i + 1] = eventSet[i];
            i--;
        }
        eventSet[i + 1] = item;
        return i + 1;
    }

private:
    double signedArea(const Point& p0, const Point& p1, const Point& p2) {
        return (p0.x - p2.x) * (p1.y - p2.y) - (p1.x - p2.x) * (p0.y - p2.y);
    }

    bool segmentCompare(SweepEvent* e1, SweepEvent* e2) {
        if (e1 == e2) {
            return false;
        }

        if (signedArea(e1->p, e1->otherSE->p, e2->p) != 0 || signedArea(e1->p, e1->otherSE->p, e2->otherSE->p) != 0) {
            if (e1->p.equals(e2->p)) {
                return e1->isBelow(e2->otherSE->p);
            }
            if (compareSweepEvent(e1, e2)) {
                return e2->isAbove(e1->p);
            }
            return e1->isBelow(e2->p);
        }

        if (e1->p.equals(e2->p)) {
            return false;
        }

        return compareSweepEvent(e1, e2);
    }

    bool compareSweepEvent(SweepEvent* e1, SweepEvent* e2) {
        if (e1->p.x > e2->p.x) {
            return true;
        }
        if (e2->p.x > e1->p.x) {
            return false;
        }
        if (!e1->p.equals(e2->p)) {
            return e1->p.y > e2->p.y;
        }
        if (e1->isLeft != e2->isLeft) {
            return e1->isLeft;
        }
        return e1->isAbove(e2->otherSE->p);
    }
};

} // namespace PolyOffset

#endif // SWEEPEVENTSET_H
