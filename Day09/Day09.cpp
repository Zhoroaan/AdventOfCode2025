#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace bg = boost::geometry;

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

void Day12(const std::string& InFileName, const int64_t InPart1Expected, const int64_t InPart2Expected)
{
    std::printf("Result for %s\n", InFileName.c_str());
    std::ifstream inputFile;
    inputFile.open(InFileName);
    std::string inputLine;
    int64_t part1Count = 0;
    int64_t part2Count = 0;
    std::vector<std::pair<int64_t, int64_t>> rectStartEndPoints;
    
    while (inputFile.is_open() && !inputFile.eof())
    {
        char delimiter = ',';
        int64_t x = -1, y = -1;
        inputFile >> x >> delimiter >> y; 
        std::getline(inputFile, inputLine);
        rectStartEndPoints.emplace_back(x, y);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    bg::model::polygon<bg::model::d2::point_xy<int>> part2Shape;
    for (const auto& point : rectStartEndPoints)
    {
        bg::append(part2Shape, bg::model::d2::point_xy<int64_t>(point.first, point.second));
    }
    bg::correct(part2Shape);
    
    for (const std::pair<int64_t, int64_t>& startPos : rectStartEndPoints)
    {
        for (const std::pair<int64_t, int64_t>& endPosition : rectStartEndPoints)
        {
            if (startPos == endPosition)
                continue;
            
            auto xDiff = std::abs(endPosition.first - startPos.first) + 1;
            auto yDiff = std::abs(endPosition.second - startPos.second) + 1;
            part1Count = std::max(xDiff * yDiff, part1Count);
            
            bg::model::polygon<bg::model::d2::point_xy<int64_t>> rectPoly;
            bg::append(rectPoly, bg::model::d2::point_xy<int64_t>(startPos.first, startPos.second));
            bg::append(rectPoly, bg::model::d2::point_xy<int64_t>(endPosition.first, startPos.second));
            bg::append(rectPoly, bg::model::d2::point_xy<int64_t>(endPosition.first, endPosition.second));
            bg::append(rectPoly, bg::model::d2::point_xy<int64_t>(startPos.first, endPosition.second));
            bg::correct(rectPoly);
            if (bg::covered_by(rectPoly, part2Shape) || bg::touches(rectPoly, part2Shape))
            {
                part2Count = std::max(xDiff * yDiff, part2Count);
            }
        }
    }
    
    
    auto end = std::chrono::high_resolution_clock::now();
    std::printf("Result part 1: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part1Count == InPart1Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::printf("Result part 2: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part2Count == InPart2Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    Day12("TestInput.txt", 50, 24);
    Day12("Input.txt", 4749929916,1572047142);
    return 0;
}
