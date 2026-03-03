#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "PointChain.h"
#include "geom/Contour.h"
#include "geom/Polygon.h"
#include <vector>

namespace PolyOffset {

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

} // namespace PolyOffset

#endif // CONNECTOR_H
