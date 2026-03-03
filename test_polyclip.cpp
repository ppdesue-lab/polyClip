#include "polyclip/PolygonClipper.h"
#include <iostream>

using namespace PolyOffset;

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
    PolygonClipper clipper(subject, clipping);
    Polygon* intersection = clipper.compute(PolygonOp::INTERSECTION);
    std::cout << "Intersection polygon has " << intersection->contours.size() << " contours" << std::endl;

    // 测试并集操作
    Polygon* unionPoly = clipper.compute(PolygonOp::UNION);
    std::cout << "Union polygon has " << unionPoly->contours.size() << " contours" << std::endl;

    // 测试差集操作
    Polygon* difference = clipper.compute(PolygonOp::DIFFERENCE);
    std::cout << "Difference polygon has " << difference->contours.size() << " contours" << std::endl;

    // 清理内存
    delete subject;
    delete clipping;
    //delete intersection;
    delete unionPoly;
    delete difference;

    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}
