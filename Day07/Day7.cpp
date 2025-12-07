#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

template <typename T>
bool IsValidIndex(const T& InContainer, int InIndex)
{
    return InIndex >= 0 && InIndex < InContainer.size();
}

int64_t ProcessPart1(std::vector<std::string> processLines)
{
    int64_t part1Count = 0;
    for (int y = 0; y < processLines.size(); ++y)
    {
        for (int x = 0; x < processLines[y].size(); ++x)
        {
            auto currentChar = processLines[y][x];
            if (currentChar == 'S' ||currentChar == '|')
            {
                if (!IsValidIndex(processLines, y + 1))
                    continue;
                
                if (processLines[y+1][x] == '^')
                {
                    bool wasSplit = false;
                    if (IsValidIndex(processLines[y+1], x - 1))
                    {
                        if (processLines[y+1][x - 1] == '.')
                        {
                            processLines[y+1][x - 1] = '|';
                            wasSplit = true;
                        }
                    }
                    if (IsValidIndex(processLines[y+1], x + 1))
                    {
                        if (processLines[y+1][x + 1] == '.')
                        {
                            processLines[y+1][x + 1] = '|';
                            wasSplit = true;
                        }
                    }
                    if (wasSplit)
                        part1Count++;
                }
                else if (IsValidIndex(processLines[y+1], x))
                    processLines[y+1][x] = '|';
            }
        }
    }
    return part1Count;
}

int64_t ProcessPart2(std::vector<std::string>& InVectorm, int x, int y, std::map<std::pair<int, int>, int64_t>& OutCachedLines)
{
    if (const bool isLastLine = y == InVectorm.size() - 1)
        return 1;
 
    if (OutCachedLines.contains(std::pair(x, y)))
        return OutCachedLines[std::pair(x, y)];
    
    int64_t result = 0;
    
    if (InVectorm[y+1][x] == '^')
    {
        //if (x >= 1) // optimized for input
            result += ProcessPart2(InVectorm, x-1, y+1, OutCachedLines);

        //if (x < InVectorm[y+1].size() - 1)  // optimized for input
            result += ProcessPart2(InVectorm, x+1, y+1, OutCachedLines);
    }
    else
        result += ProcessPart2(InVectorm, x, y+1, OutCachedLines);
    
    OutCachedLines.emplace(std::pair(x, y), result);
    return result;
}

void Day07(const std::string& InFileName, const int64_t InPart1Expected, const int64_t InPart2Expected)
{
    std::printf("Result for %s\n", InFileName.c_str());
    std::ifstream inputFile;
    inputFile.open(InFileName);
    std::string inputLine;
    
    
    std::vector<std::string> processLines;
    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        processLines.emplace_back(std::move(inputLine));
    }
    
    std::vector<std::string> processLinesPart2 = processLines;
    auto start = std::chrono::high_resolution_clock::now();
    int64_t part1Count = ProcessPart1(processLines);
    auto foundItr = std::find(processLines[0].begin(), processLines[0].end(), 'S');
    std::map<std::pair<int, int>, int64_t> cachedLines;
    int64_t part2Count = ProcessPart2(processLinesPart2, foundItr - processLines[0].begin() , 1, cachedLines);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::printf("Result part 1: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part1Count == InPart1Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::printf("Result part 2: %s%lld%s which is %s%s%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET, ANSI_BOLD_UNDERLINE, part2Count == InPart2Expected ? "Correct" : "Incorrect", ANSI_RESET);
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    Day07("TestInput.txt", 21,40);
    Day07("Input.txt", 1594, 15650261281478);
    return 0;
}
