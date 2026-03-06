#define _USE_MATH_DEFINES
#include <cmath>
#include "PolyClip.hpp"
#include <iostream>

using namespace PolyOffset;

void printPolygonInfo(const char* operation, Polygon* polygon) {
    std::cout << operation << " polygon has " << polygon->contours.size() << " contours" << std::endl;
    for (size_t i = 0; i < polygon->contours.size(); i++) {
        Contour* contour = polygon->contours[i];
        std::cout << "  Contour " << i << " has " << contour->points.size() << " points" << std::endl;
    }
}

int main() {
    // 创建两个简单的多边形
    Polygon* subject = new Polygon();
    Contour* subjectContour = new Contour();
    subjectContour->add(Point(0, 0));
    subjectContour->add(Point(100, 0));
    subjectContour->add(Point(100, 100));
    subjectContour->add(Point(0, 100));
    subject->addContour(subjectContour);

    Polygon* clipping = new Polygon();
    Contour* clippingContour = new Contour();
    clippingContour->add(Point(50, 50));
    clippingContour->add(Point(150, 50));
    clippingContour->add(Point(150, 150));
    clippingContour->add(Point(50, 150));
    clipping->addContour(clippingContour);

    // 测试交集操作
    PolygonClipper clipper1(subject, clipping);
    Polygon* intersection = clipper1.compute(PolygonOp::INTERSECTION);
    printPolygonInfo("Intersection", intersection);

    // 测试并集操作
    PolygonClipper clipper2(subject, clipping);
    Polygon* unionPoly = clipper2.compute(PolygonOp::UNION);
    printPolygonInfo("Union", unionPoly);

    // 测试差集操作
    PolygonClipper clipper3(subject, clipping);
    Polygon* difference = clipper3.compute(PolygonOp::DIFFERENCE);
    printPolygonInfo("Difference", difference);

    // 清理内存
    delete subject;
    delete clipping;
    delete intersection;
    delete unionPoly;
    delete difference;

    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}