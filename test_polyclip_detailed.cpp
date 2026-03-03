#define _USE_MATH_DEFINES
#include <cmath>
#include "polyclip/PolygonClipper.h"
#include <iostream>

using namespace PolyOffset;

void printPolygonInfo(const char* operation, Polygon* polygon) {
    std::cout << operation << " polygon has " << polygon->contours.size() << " contours" << std::endl;
    for (size_t i = 0; i < polygon->contours.size(); i++) {
        Contour* contour = polygon->contours[i];
        std::cout << "  Contour " << i << " has " << contour->points.size() << " points:" << std::endl;
        for (size_t j = 0; j < contour->points.size(); j++) {
            Point p = contour->points[j];
            std::cout << "    (" << p.x << ", " << p.y << ")" << std::endl;
        }
    }
}

int main() {
    // 创建一个正方形作为主题多边形
    Polygon* subject = new Polygon();
    Contour* subjectContour = new Contour();
    subjectContour->add(Point(0, 0));
    subjectContour->add(Point(100, 0));
    subjectContour->add(Point(100, 100));
    subjectContour->add(Point(0, 100));
    subject->addContour(subjectContour);

    // 创建一个圆形作为裁剪多边形（使用多边形近似）
    Polygon* clipping = new Polygon();
    Contour* clippingContour = new Contour();
    int numPoints = 32;
    double radius = 30;
    double centerX = 50;
    double centerY = 50;
    for (int i = 0; i < numPoints; i++) {
        double angle = 2 * M_PI * i / numPoints;
        double x = centerX + radius * cos(angle);
        double y = centerY + radius * sin(angle);
        clippingContour->add(Point(x, y));
    }
    clipping->addContour(clippingContour);

    // 测试交集操作
    PolygonClipper clipper(subject, clipping);
    Polygon* intersection = clipper.compute(PolygonOp::INTERSECTION);
    printPolygonInfo("Intersection", intersection);

    // 测试并集操作
    Polygon* unionPoly = clipper.compute(PolygonOp::UNION);
    printPolygonInfo("Union", unionPoly);

    // 测试差集操作
    Polygon* difference = clipper.compute(PolygonOp::DIFFERENCE);
    printPolygonInfo("Difference", difference);

    // 清理内存
    delete subject;
    delete clipping;
    delete intersection;
    delete unionPoly;
    delete difference;

    std::cout << "Detailed test completed successfully!" << std::endl;
    return 0;
}
