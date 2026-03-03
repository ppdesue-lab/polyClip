#ifndef EVENTQUEUE_H
#define EVENTQUEUE_H

#include "SweepEvent.h"
#include <vector>
#include <algorithm>

namespace PolyOffset {

class EventQueue {
private:
    std::vector<SweepEvent*> elements;
    bool sorted;

    int compareSweepEvent(SweepEvent* e1, SweepEvent* e2) {
        if (e1->p.x > e2->p.x) {
            return -1;
        }
        if (e2->p.x > e1->p.x) {
            return 1;
        }
        if (!e1->p.equals(e2->p)) {
            return (e1->p.y > e2->p.y) ? -1 : 1;
        }
        if (e1->isLeft != e2->isLeft) {
            return (e1->isLeft) ? -1 : 1;
        }
        return e1->isAbove(e2->otherSE->p) ? -1 : 1;
    }

    static bool compareSweepEventStatic(SweepEvent* e1, SweepEvent* e2) {
        EventQueue eq;
        return eq.compareSweepEvent(e1, e2) < 0;
    }

public:
    EventQueue() : sorted(false) {}

    void enqueue(SweepEvent* obj) {
        if (sorted) {
            int length = elements.size();
            if (length == 0) {
                elements.push_back(obj);
                return;
            }

            elements.push_back(nullptr);

            int i = length - 1;
            while (i >= 0 && compareSweepEvent(obj, elements[i]) == -1) {
                elements[i + 1] = elements[i];
                i--;
            }
            elements[i + 1] = obj;
        } else {
            elements.push_back(obj);
        }
    }

    SweepEvent* dequeue() {
        if (!sorted) {
            sorted = true;
            std::sort(elements.begin(), elements.end(), compareSweepEventStatic);
        }

        SweepEvent* event = elements.back();
        elements.pop_back();
        return event;
    }

    void clear()
    {
        elements.clear();
    }

    bool isEmpty() {
        return elements.empty();
    }

    ~EventQueue() {
        for (SweepEvent* event : elements) {
            delete event;
        }
    }
};

} // namespace PolyOffset

#endif // EVENTQUEUE_H
