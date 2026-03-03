#ifndef POLYGON_H
#define POLYGON_H

#include "Contour.h"

namespace PolyOffset {

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

} // namespace PolyOffset

#endif // POLYGON_H
