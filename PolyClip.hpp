#ifndef POLYCLIP_HPP
#define POLYCLIP_HPP

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <vector>

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

class EdgeType {
public:
    static const int NORMAL = 0;
    static const int NON_CONTRIBUTING = 1;
    static const int SAME_TRANSITION = 2;
    static const int DIFFERENT_TRANSITION = 3;
};

class PolygonOp {
public:
    static const int UNION = 0;
    static const int INTERSECTION = 1;
    static const int DIFFERENCE = 2;
};

class Rectangle {
public:
    double x, y, width, height;

    Rectangle() : x(0), y(0), width(0), height(0) {}
    Rectangle(double x, double y, double width, double height) 
        : x(x), y(y), width(width), height(height) {}

    double getRight() const {
        return x + width;
    }

    double getBottom() const {
        return y + height;
    }

    bool intersects(const Rectangle& other) const {
        return !(getRight() < other.x || x > other.getRight() || 
                 getBottom() < other.y || y > other.getBottom());
    }

    Rectangle unionRect(const Rectangle& other) const {
        double minX = std::min(x, other.x);
        double minY = std::min(y, other.y);
        double maxX = std::max(getRight(), other.getRight());
        double maxY = std::max(getBottom(), other.getBottom());
        return Rectangle(minX, minY, maxX - minX, maxY - minY);
    }
};

class Segment {
public:
    Point start;
    Point end;

    Segment() : start(), end() {}
    Segment(const Point& start, const Point& end) : start(start), end(end) {}
};

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

class Polygon {
public:
    std::vector<Contour*> contours;
    Rectangle* bounds;

    Polygon() : bounds(nullptr) {}

    int numVertices() {
        int verticesCount = 0;
        for (Contour* c : contours) {
            verticesCount += c->points.size();
        }
        return verticesCount;
    }

    Rectangle boundingBox() {
        if (bounds) {
            return *bounds;
        }

        Rectangle* bb = nullptr;
        for (Contour* c : contours) {
            Rectangle cBB = c->boundingBox();
            if (!bb) {
                bb = new Rectangle(cBB.x, cBB.y, cBB.width, cBB.height);
            } else {
                *bb = bb->unionRect(cBB);
            }
        }

        bounds = bb;
        return *bounds;
    }

    void addContour(Contour* c) {
        contours.push_back(c);
    }

    Polygon* clone() {
        Polygon* poly = new Polygon();
        for (Contour* cont : contours) {
            Contour* c = new Contour();
            for (const Point& p : cont->points) {
                c->add(Point(p.x, p.y));
            }
            poly->addContour(c);
        }
        return poly;
    }

    ~Polygon() {
        for (Contour* c : contours) {
            delete c;
        }
        if (bounds) {
            delete bounds;
        }
    }
};

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

class Connector {
private:
    std::vector<PointChain*> openPolygons;
    std::vector<PointChain*> closedPolygons;

public:
    Connector() {}

    void add(const Segment& s) {
        for (size_t j = 0; j < openPolygons.size(); j++) {
            PointChain* chain = openPolygons[j];
            if (chain->linkSegment(s)) {
                if (chain->closed) {
                    if (chain->pointList.size() == 2) {
                        chain->closed = false;
                        return;
                    }
                    openPolygons.erase(openPolygons.begin() + j);
                    closedPolygons.push_back(chain);
                } else {
                    for (size_t i = j + 1; i < openPolygons.size(); i++) {
                        if (chain->linkPointChain(openPolygons[i])) {
                            openPolygons.erase(openPolygons.begin() + i);
                            break;
                        }
                    }
                }
                return;
            }
        }

        PointChain* newChain = new PointChain(s);
        openPolygons.push_back(newChain);
    }

    Polygon* toPolygon() {
        Polygon* polygon = new Polygon();
        for (PointChain* pointChain : closedPolygons) {
            Contour* c = new Contour();
            for (const Point& p : pointChain->pointList) {
                c->add(p);
            }
            polygon->addContour(c);
        }
        return polygon;
    }

    ~Connector() {
        for (PointChain* chain : openPolygons) {
            delete chain;
        }
        for (PointChain* chain : closedPolygons) {
            delete chain;
        }
    }
};

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

#endif // POLYCLIP_HPP
