#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

struct Machine
{
    std::vector<std::string> Connections;
    std::vector<int32_t> FastConnections;
    std::string Id;
};

int64_t FindPaths(const std::string& InFrom, const std::string& InTarget,
    const std::vector<Machine>& InMachines, std::map<std::string, int32_t>& InOutCache)
{
    int64_t returnCount = 0;
    if (InFrom == "out")
        return 0;
    
    if (InOutCache.contains(InFrom))
        return InOutCache[InFrom];

    for (const auto& machine : InMachines)
    {
        if (machine.Id == InFrom)
        {
            for (const auto& childId : machine.Connections)
            {
                if (childId == InTarget)
                {
                    returnCount++;
                    continue;
                }
                returnCount += FindPaths(childId, InTarget, InMachines, InOutCache);
            }
            break;
        }
    }
    InOutCache.insert({InFrom, returnCount});

    return returnCount;
}

int64_t FindPathsWithCache(const std::string& InFrom,
                                const std::string& InGoal,
                                const std::vector<Machine>& InMachines)
{
    std::map<std::string, int32_t> cache;
    return FindPaths(InFrom, InGoal, InMachines, cache);
}

void Day12(const std::string& InFileName, const int64_t InPart1Expected, const int64_t InPart2Expected)
{
    std::printf("Result for %s\n", InFileName.c_str());
    std::ifstream inputFile;
    inputFile.open(InFileName);
    std::string inputLine;
    int64_t part1Count = 0;
    int64_t part2Count = 0;
    
    std::vector<Machine> machines;
    
    while (inputFile.is_open() && !inputFile.eof())
    {
        Machine machine;
        std::getline(inputFile, machine.Id, ':');
        std::getline(inputFile, inputLine);
        std::stringstream ss(inputLine);
        std::string token;

        while (std::getline(ss, token, ' ')) {
            if (!token.empty())
                machine.Connections.emplace_back(token);
        }
        machines.emplace_back(machine);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    int32_t startId = -1;
    
    part1Count = FindPathsWithCache("you", "out", machines);
    std::map<std::string, int32_t> a;
    int64_t fftToDac = FindPathsWithCache("svr", "fft", machines)
        * FindPathsWithCache("fft", "dac", machines)
        * FindPathsWithCache("dac", "out", machines);
    int64_t dactoFFT = FindPathsWithCache("svr", "dac", machines)
        * FindPathsWithCache("dac", "fft", machines)
        * FindPathsWithCache("fft", "out", machines);
    
    part2Count = fftToDac + dactoFFT;
    
    auto end = std::chrono::high_resolution_clock::now();
    std::printf("Result part 1: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part1Count == InPart1Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::printf("Result part 2: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part2Count == InPart2Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    Day12("TestInput.txt", 0, 2); // Part 1 breaks with new Test Input
    Day12("Input.txt", 772,423227545768872);
    return 0;
}
