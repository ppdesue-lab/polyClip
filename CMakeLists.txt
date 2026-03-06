cmake_minimum_required(VERSION 3.10)
project(polyclip VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(polyclip)

set(SOURCE_FILES
    polyclip/Point.h
    polyclip/Rectangle.h
    polyclip/geom/Contour.h
    polyclip/geom/Polygon.h
    polyclip/geom/Segment.h
    polyclip/sweepline/EventQueue.h
    polyclip/sweepline/SweepEvent.h
    polyclip/sweepline/SweepEventSet.h
    polyclip/Connector.h
    polyclip/EdgeType.h
    polyclip/PointChain.h
    polyclip/PolygonClipper.h
    polyclip/PolygonOp.h
)

add_library(polyclip INTERFACE)
target_include_directories(polyclip INTERFACE polyclip)

# 创建测试可执行文件
add_executable(test_polyclip test_polyclip.cpp)
target_link_libraries(test_polyclip polyclip)

# 创建详细测试可执行文件
add_executable(test_polyclip_detailed test_polyclip_detailed.cpp)
target_link_libraries(test_polyclip_detailed polyclip)

# 创建单一头文件测试可执行文件
add_executable(test_single_header test_single_header.cpp)
