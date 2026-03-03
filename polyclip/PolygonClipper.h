#ifndef POLYGONCLIPPER_H
#define POLYGONCLIPPER_H

#include "geom/Polygon.h"
#include "sweepline/EventQueue.h"
#include "sweepline/SweepEventSet.h"
#include "Connector.h"
#include "EdgeType.h"
#include "PolygonOp.h"
#include <algorithm>

namespace PolyOffset {

class PolygonClipper {
private:
    Polygon* subject;
    Polygon* clipping;
    EventQueue* eventQueue;

    static const int SUBJECT = 0;
    static const int CLIPPING = 1;

    std::vector<double> findIntersection(const Segment& seg0, const Segment& seg1) {
        Point pi0, pi1;
        Point p0 = seg0.start;
        Point d0(seg0.end.x - p0.x, seg0.end.y - p0.y);
        Point p1 = seg1.start;
        Point d1(seg1.end.x - p1.x, seg1.end.y - p1.y);
        double sqrEpsilon = 0.0000001;
        Point E(p1.x - p0.x, p1.y - p0.y);
        double kross = d0.x * d1.y - d0.y * d1.x;
        double sqrKross = kross * kross;
        double sqrLen0 = d0.length() * d0.length();
        double sqrLen1 = d1.length() * d1.length();

        if (sqrKross > sqrEpsilon * sqrLen0 * sqrLen1) {
            double s = (E.x * d1.y - E.y * d1.x) / kross;
            if (s < 0 || s > 1) {
                return {0, 0, 0, 0, 0};
            }
            double t = (E.x * d0.y - E.y * d0.x) / kross;
            if (t < 0 || t > 1) {
                return {0, 0, 0, 0, 0};
            }
            pi0.x = p0.x + s * d0.x;
            pi0.y = p0.y + s * d0.y;
            return {1, pi0.x, pi0.y, 0, 0};
        }

        double sqrLenE = E.length() * E.length();
        kross = E.x * d0.y - E.y * d0.x;
        sqrKross = kross * kross;
        if (sqrKross > sqrEpsilon * sqrLen0 * sqrLenE) {
            return {0, 0, 0, 0, 0};
        }

        double s0 = (d0.x * E.x + d0.y * E.y) / sqrLen0;
        double s1 = s0 + (d0.x * d1.x + d0.y * d1.y) / sqrLen0;
        double smin = std::min(s0, s1);
        double smax = std::max(s0, s1);
        std::vector<double> w;
        int imax = findIntersection2(0.0, 1.0, smin, smax, w);

        if (imax > 0) {
            pi0.x = p0.x + w[0] * d0.x;
            pi0.y = p0.y + w[0] * d0.y;
            if (imax > 1) {
                pi1.x = p0.x + w[1] * d0.x;
                pi1.y = p0.y + w[1] * d0.y;
                return {2, pi0.x, pi0.y, pi1.x, pi1.y};
            }
            return {1, pi0.x, pi0.y, 0, 0};
        }

        return {0, 0, 0, 0, 0};
    }

    int findIntersection2(double u0, double u1, double v0, double v1, std::vector<double>& w) {
        if (u1 < v0 || u0 > v1) {
            return 0;
        }
        if (u1 > v0) {
            if (u0 < v1) {
                w.push_back((u0 < v0) ? v0 : u0);
                w.push_back((u1 > v1) ? v1 : u1);
                return 2;
            } else {
                w.push_back(u0);
                return 1;
            }
        } else {
            w.push_back(u1);
            return 1;
        }
    }

    bool sec(SweepEvent* e1, SweepEvent* e2) {
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

    void possibleIntersection(SweepEvent* e1, SweepEvent* e2) {
        std::vector<double> intData = findIntersection(e1->segment(), e2->segment());
        int numIntersections = static_cast<int>(intData[0]);
        Point ip1(intData[1], intData[2]);
        Point ip2(intData[3], intData[4]);

        if (numIntersections == 0) {
            return;
        }

        if (numIntersections == 1 && (e1->p.equals(e2->p) || e1->otherSE->p.equals(e2->otherSE->p))) {
            return;
        }

        if (numIntersections == 2 && e1->p.equals(e2->p)) {
            return;
        }

        if (numIntersections == 1) {
            if (!e1->p.equals(ip1) && !e1->otherSE->p.equals(ip1)) {
                divideSegment(e1, ip1);
            }
            if (!e2->p.equals(ip1) && !e2->otherSE->p.equals(ip1)) {
                divideSegment(e2, ip1);
            }
            return;
        }

        std::vector<SweepEvent*> sortedEvents;
        if (e1->p.equals(e2->p)) {
            sortedEvents.push_back(nullptr);
        } else if (sec(e1, e2)) {
            sortedEvents.push_back(e2);
            sortedEvents.push_back(e1);
        } else {
            sortedEvents.push_back(e1);
            sortedEvents.push_back(e2);
        }

        if (e1->otherSE->p.equals(e2->otherSE->p)) {
            sortedEvents.push_back(nullptr);
        } else if (sec(e1->otherSE, e2->otherSE)) {
            sortedEvents.push_back(e2->otherSE);
            sortedEvents.push_back(e1->otherSE);
        } else {
            sortedEvents.push_back(e1->otherSE);
            sortedEvents.push_back(e2->otherSE);
        }

        if (sortedEvents.size() == 2) {
            e1->edgeType = e1->otherSE->edgeType = EdgeType::NON_CONTRIBUTING;
            e2->edgeType = e2->otherSE->edgeType = (e1->inOut == e2->inOut) ? EdgeType::SAME_TRANSITION : EdgeType::DIFFERENT_TRANSITION;
            return;
        }

        if (sortedEvents.size() == 3) {
            sortedEvents[1]->edgeType = sortedEvents[1]->otherSE->edgeType = EdgeType::NON_CONTRIBUTING;
            if (sortedEvents[0]) {
                sortedEvents[0]->otherSE->edgeType = (e1->inOut == e2->inOut) ? EdgeType::SAME_TRANSITION : EdgeType::DIFFERENT_TRANSITION;
            } else {
                sortedEvents[2]->otherSE->edgeType = (e1->inOut == e2->inOut) ? EdgeType::SAME_TRANSITION : EdgeType::DIFFERENT_TRANSITION;
            }
            divideSegment(sortedEvents[0] ? sortedEvents[0] : sortedEvents[2]->otherSE, sortedEvents[1]->p);
            return;
        }

        if (sortedEvents[0] != sortedEvents[3]->otherSE) {
            sortedEvents[1]->edgeType = EdgeType::NON_CONTRIBUTING;
            sortedEvents[2]->edgeType = (e1->inOut == e2->inOut) ? EdgeType::SAME_TRANSITION : EdgeType::DIFFERENT_TRANSITION;
            divideSegment(sortedEvents[0], sortedEvents[1]->p);
            divideSegment(sortedEvents[1], sortedEvents[2]->p);
            return;
        }

        sortedEvents[1]->edgeType = sortedEvents[1]->otherSE->edgeType = EdgeType::NON_CONTRIBUTING;
        divideSegment(sortedEvents[0], sortedEvents[1]->p);
        sortedEvents[3]->otherSE->edgeType = (e1->inOut == e2->inOut) ? EdgeType::SAME_TRANSITION : EdgeType::DIFFERENT_TRANSITION;
        divideSegment(sortedEvents[3]->otherSE, sortedEvents[2]->p);
    }

    void divideSegment(SweepEvent* e, const Point& p) {
        SweepEvent* r = new SweepEvent(p, false, e->polygonType, e, e->edgeType);
        SweepEvent* l = new SweepEvent(p, true, e->polygonType, e->otherSE, e->otherSE->edgeType);

        if (sec(l, e->otherSE)) {
            e->otherSE->isLeft = true;
            e->isLeft = false;
        }

        e->otherSE->otherSE = l;
        e->otherSE = r;

        eventQueue->enqueue(l);
        eventQueue->enqueue(r);
    }

    void processSegment(const Segment& segment, int polyType) {
        if (segment.start.equals(segment.end)) {
            return;
        }

        SweepEvent* e1 = new SweepEvent(segment.start, true, polyType);
        SweepEvent* e2 = new SweepEvent(segment.end, true, polyType, e1);
        e1->otherSE = e2;

        if (e1->p.x < e2->p.x) {
            e2->isLeft = false;
        } else if (e1->p.x > e2->p.x) {
            e1->isLeft = false;
        } else if (e1->p.y < e2->p.y) {
            e2->isLeft = false;
        } else {
            e1->isLeft = false;
        }

        eventQueue->enqueue(e1);
        eventQueue->enqueue(e2);
    }

public:
    PolygonClipper(Polygon* subject, Polygon* clipping) {
        this->subject = subject;
        this->clipping = clipping;
        eventQueue = new EventQueue();
    }

    Polygon* compute(int operation) {
        Polygon* result = nullptr;
        
        eventQueue->clear();
        if (subject->contours.empty() || clipping->contours.empty()) {
            if (operation == PolygonOp::DIFFERENCE) {
                result = subject;
            } else if (operation == PolygonOp::UNION) {
                result = subject->contours.empty() ? clipping : subject;
            }
            return result;
        }

        Rectangle subjectBB = subject->boundingBox();
        Rectangle clippingBB = clipping->boundingBox();

        if (!subjectBB.intersects(clippingBB)) {
            if (operation == PolygonOp::DIFFERENCE) {
                result = subject;
            } else if (operation == PolygonOp::UNION) {
                result = subject;
                for (Contour* c : clipping->contours) {
                    result->addContour(c);
                }
            }
            return result;
        }

        for (Contour* sCont : subject->contours) {
            for (size_t pParse1 = 0; pParse1 < sCont->points.size(); pParse1++) {
                processSegment(sCont->getSegment(pParse1), SUBJECT);
            }
        }

        for (Contour* cCont : clipping->contours) {
            for (size_t pParse2 = 0; pParse2 < cCont->points.size(); pParse2++) {
                processSegment(cCont->getSegment(pParse2), CLIPPING);
            }
        }

        Connector* connector = new Connector();
        SweepEventSet* S = new SweepEventSet();

        SweepEvent* e;
        double MINMAX_X = std::min(subjectBB.getRight(), clippingBB.getRight());

        SweepEvent* prev = nullptr;
        SweepEvent* next = nullptr;

        while (!eventQueue->isEmpty()) {
            prev = nullptr;
            next = nullptr;

            e = eventQueue->dequeue();

            if ((operation == PolygonOp::INTERSECTION && e->p.x > MINMAX_X) || 
                (operation == PolygonOp::DIFFERENCE && e->p.x > subjectBB.getRight())) {
                return connector->toPolygon();
            }

            if (operation == PolygonOp::UNION && e->p.x > MINMAX_X) {
                if (!e->isLeft) {
                    connector->add(e->segment());
                }

                while (!eventQueue->isEmpty()) {
                    e = eventQueue->dequeue();
                    if (!e->isLeft) {
                        connector->add(e->segment());
                    }
                }

                return connector->toPolygon();
            }

            if (e->isLeft) {
                int pos = S->insert(e);

                prev = (pos > 0) ? S->eventSet[pos - 1] : nullptr;
                next = (pos < static_cast<int>(S->eventSet.size()) - 1) ? S->eventSet[pos + 1] : nullptr;

                if (!prev) {
                    e->inside = e->inOut = false;
                } else if (prev->edgeType != EdgeType::NORMAL) {
                    if (pos - 2 < 0) {
                        e->inside = e->inOut = false;
                        if (prev->polygonType != e->polygonType) {
                            e->inside = true;
                        } else {
                            e->inOut = true;
                        }
                    } else {
                        SweepEvent* prevTwo = S->eventSet[pos - 2];
                        if (prev->polygonType == e->polygonType) {
                            e->inOut = !prev->inOut;
                            e->inside = !prevTwo->inOut;
                        } else {
                            e->inOut = !prevTwo->inOut;
                            e->inside = !prev->inOut;
                        }
                    }
                } else if (e->polygonType == prev->polygonType) {
                    e->inside = prev->inside;
                    e->inOut = !prev->inOut;
                } else {
                    e->inside = !prev->inOut;
                    e->inOut = prev->inside;
                }

                if (next) {
                    possibleIntersection(e, next);
                }

                if (prev) {
                    possibleIntersection(e, prev);
                }
            } else {
                auto it = std::find(S->eventSet.begin(), S->eventSet.end(), e->otherSE);
                int otherPos = (it != S->eventSet.end()) ? std::distance(S->eventSet.begin(), it) : -1;

                if (otherPos != -1) {
                    prev = (otherPos > 0) ? S->eventSet[otherPos - 1] : nullptr;
                    next = (otherPos < static_cast<int>(S->eventSet.size()) - 1) ? S->eventSet[otherPos + 1] : nullptr;
                }

                switch (e->edgeType) {
                    case EdgeType::NORMAL:
                        switch (operation) {
                            case PolygonOp::INTERSECTION:
                                if (e->otherSE->inside) {
                                    connector->add(e->segment());
                                }
                                break;
                            case PolygonOp::UNION:
                                if (!e->otherSE->inside) {
                                    connector->add(e->segment());
                                }
                                break;
                            case PolygonOp::DIFFERENCE:
                                if (((e->polygonType == SUBJECT) && (!e->otherSE->inside)) || 
                                    (e->polygonType == CLIPPING && e->otherSE->inside)) {
                                    connector->add(e->segment());
                                }
                                break;
                        }
                        break;
                    case EdgeType::SAME_TRANSITION:
                        if (operation == PolygonOp::INTERSECTION || operation == PolygonOp::UNION) {
                            connector->add(e->segment());
                        }
                        break;
                    case EdgeType::DIFFERENT_TRANSITION:
                        if (operation == PolygonOp::DIFFERENCE) {
                            connector->add(e->segment());
                        }
                        break;
                }

                if (otherPos != -1) {
                    S->remove(S->eventSet[otherPos]);
                }

                if (next && prev) {
                    possibleIntersection(next, prev);
                }
            }
        }

        return connector->toPolygon();
    }

    ~PolygonClipper() {
        delete eventQueue;
    }
};

} // namespace PolyOffset

#endif // POLYGONCLIPPER_H
