#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

int64_t CalculatePart1(const std::vector<std::vector<int64_t>>& InNumbers, const std::vector<char>& InOps)
{
    int64_t part1Count = 0;
    for (int index = 0; index < InOps.size(); ++index)
    {
        int64_t result = 0;
        for (const auto& numberLine : InNumbers)
        {
            if (InOps[index] == '+')
                result += numberLine[index];
            else
            {
                if (result != 0)
                    result *= numberLine[index];
                else
                    result = numberLine[index];
            }
        }
        part1Count += result;
    }
    return part1Count;
}

int64_t CalculatePart2(const std::vector<std::string>& InAllLines)
{
    int64_t part2Count = 0;
    int maxRowLength = 0;
    for (auto& line : InAllLines)
    {
        maxRowLength = std::max<std::basic_string<char>::size_type>(line.size(), maxRowLength);
    }
    char lastOp;
    int64_t currentResult = 0;
    for (size_t i = 0; i < maxRowLength; ++i)
    {        
        if (InAllLines.back().size() > i && InAllLines.back()[i] != ' ')
        {
            lastOp = InAllLines.back()[i];
        }
        
        bool allEmpty = std::all_of(InAllLines.begin(), InAllLines.end(), [i](const auto& InLine)
        {
            return InLine[i] == ' ';
        });
        
        if (!allEmpty)
        {
            int64_t currentNumber = 0;
            for (const auto& line : InAllLines)
            {
                const bool isOpLine = line == InAllLines.back();
                if (isOpLine)
                    break;
                
                if (line.size() < i)
                    continue;
                
                if (line[i] != ' ')
                {
                    currentNumber *= 10;
                    currentNumber += line[i] - '0';
                }
            }
            if (currentNumber != 0)
            {
                if (lastOp == '+')
                    currentResult += currentNumber;
                else
                {
                    if (currentResult != 0)
                        currentResult *= currentNumber;
                    else
                        currentResult = currentNumber;
                }
            }
        }

        if (allEmpty || i == maxRowLength - 1)
        {
            part2Count += currentResult;
            currentResult = 0;
        }
    }
    return part2Count;
}

void Day07(const std::string& InFileName)
{
    std::printf("Result for %s\n", InFileName.c_str());
    std::ifstream inputFile;
    inputFile.open(InFileName);
    std::string inputLine;
    
    int64_t part1Count = 0;
    int64_t part2Count = 0;
    
    bool readOps = false;
    std::vector<std::vector<int64_t>> numbers;
    std::vector<std::string> allLines;
    std::vector<char> ops;
    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        allLines.emplace_back(inputLine);
        std::regex e(R"(\d+|[\\*\\+])");
        
        std::sregex_iterator iter(inputLine.begin(), inputLine.end(), e);
        std::sregex_iterator end;
        
        if (iter == end)
            continue;
        
        if ((*iter)[0] == '+' || (*iter)[0] == '*')
            readOps = true;
        
        std::vector<int64_t> currentNumberLine;
        
        while(iter != end)
        {
            for(unsigned i = 0; i < iter->size(); ++i)
            {
                if (readOps)
                    ops.emplace_back((*iter)[0].str()[0]);
                else
                {
                    currentNumberLine.emplace_back(atoll((*iter)[0].str().c_str()));
                }
            }
            ++iter;
        }
        if (!currentNumberLine.empty())
        {
            numbers.emplace_back(std::move(currentNumberLine));
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    part1Count = CalculatePart1(numbers, ops);
    part2Count = CalculatePart2(allLines);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::printf("Result part 1: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET);
    std::printf("Result part 2: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET);
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    Day07("TestInput.txt");
    Day07("Input.txt");
    return 0;
}
