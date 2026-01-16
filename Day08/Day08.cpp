#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <ranges>
#include <unordered_map>
#include <numeric>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

class FVector
{
public:
    FVector() = default;
    FVector(double InX, double InY, double InZ)
        : X(InX), Y(InY), Z(InZ) {}

    double Distance2 (const FVector& InOther) const
    {
        return std::pow(InOther.X - X, 2) 
        + std::pow(InOther.Y - Y, 2)
        + std::pow(InOther.Z - Z, 2);
    }

    bool operator==(const FVector& InOtherVector) const
    {
        return X == InOtherVector.X && Y == InOtherVector.Y && Z == InOtherVector.Z;
    }
    double X = -1;
    double Y = -1;
    double Z = -1;
    int32_t Circuit = -1;
};

struct DSU {
    std::vector<int> parent;

    DSU(size_t n) : parent(n) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int v) {
        if (parent[v] == v) return v;
        return parent[v] = find(parent[v]);
    }

    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a != b) parent[b] = a;
    }
};

void Day12(const std::string& InFileName, const int64_t InPart1Expected, const int64_t InPart2Expected)
{
    std::printf("Result for %s\n", InFileName.c_str());
    std::ifstream inputFile;
    inputFile.open(InFileName);
    std::string inputLine;
    
    std::vector<FVector> inputVectors;
    
    while (inputFile.is_open() && !inputFile.eof())
    {
        FVector newVector;
        char delimiter;
        inputFile >> newVector.X >> delimiter >> newVector.Y >> delimiter >> newVector.Z;
        std::getline(inputFile, inputLine);
        inputVectors.emplace_back(newVector);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    int64_t part1Count = 0;
    int64_t part2Count = 0;

    auto pairs = inputVectors 
        | std::views::transform([&](const FVector& a) {
              return inputVectors
                  | std::views::filter([&](const FVector& b){ return &a < &b; }) // only keep one ordering
                  | std::views::transform([&](const FVector& b) {
                        return std::tuple{a, b, a.Distance2(b)};
                    });
          })
        | std::views::join  // flatten nested ranges
        | std::views::filter([](const std::tuple<FVector, FVector, double>& InCheck){return std::get<2>(InCheck) != 0;})
        | std::ranges::to<std::vector<std::tuple<FVector, FVector, double>>>();
    
    std::ranges::sort(pairs, [](const std::tuple<FVector, FVector, double>& InVec1, const std::tuple<FVector, FVector, double>& InVec2)
        { return std::get<2>(InVec1) < std::get<2>(InVec2); });
    
    pairs = pairs
    | std::views::filter([](auto&& tup){ return std::get<2>(tup) != 0; })
    | std::views::take(10)
    | std::ranges::to<std::vector<std::tuple<FVector, FVector, double>>>();
    
    // pairs is vector<tuple<FVector,FVector,double>>
    DSU dsu(inputVectors.size());

    // Map FVector* back to index
    auto indexOf = [&](const FVector& v) {
        return static_cast<int>(&v - &inputVectors[0]);
    };

    for (auto& [a,b,d] : pairs) {
        dsu.unite(indexOf(a), indexOf(b));
    }

    // Group by root
    std::unordered_map<int, std::vector<FVector>> circuits;
    for (auto& [a,b,d] : pairs) {
        circuits[dsu.find(indexOf(a))].push_back(a);
        circuits[dsu.find(indexOf(b))].push_back(b);
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
    Day12("TestInput.txt", 40, 25272);
    //Day07("Input.txt", 66640,78894156);
    return 0;
}
