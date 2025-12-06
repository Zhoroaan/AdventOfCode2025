#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

void Day06(const std::string& InFilname)
{
    std::printf("Result for %s\n", InFilname.c_str());
    std::ifstream inputFile;
    inputFile.open(InFilname);
    std::string inputLine;
    
    int64_t part1Count = 0;
    int64_t part2Count = 0;
    
    std::vector<std::pair<int64_t, int64_t>> nonSpoiledRanges;
    

    bool readingRanged = true;
    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        if (readingRanged && inputLine.size() == 0)
        {
            readingRanged = false;
            continue;
        }
        if (readingRanged)
        {
            int64_t start, end;
            sscanf_s(inputLine.c_str(), "%lld-%lld", &start, &end, inputLine.size());
            if (start < end)
                nonSpoiledRanges.emplace_back(start, end);
            else
                nonSpoiledRanges.emplace_back(end, start);
        }
        else
        {
            int64_t checkSpoled;
            sscanf_s(inputLine.c_str(), "%lld", &checkSpoled, inputLine.size());
            bool isSpoiled = true;
            for (const auto& range : nonSpoiledRanges)
            {
                if (checkSpoled >= range.first && checkSpoled <= range.second)
                {
                    isSpoiled = false;
                    break;
                }
            }
            if (!isSpoiled)
            {
                part1Count++;
            }
        }
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(nonSpoiledRanges.begin(), nonSpoiledRanges.end(), 
        [](const std::pair<int64_t, int64_t>& InLHS, const std::pair<int64_t, int64_t>& InRHS)
    {
       return InLHS.first < InRHS.first || (InLHS.first == InRHS.first && InLHS.second < InRHS.second); 
    });
    
    int64_t numMoves = 0;
    do
    {
        numMoves = 0;
        for (int index = 0; index < nonSpoiledRanges.size(); ++index)
        {
            for (int indexCompare = index + 1; indexCompare < nonSpoiledRanges.size(); ++indexCompare)
            {
                if (nonSpoiledRanges[index].first <= nonSpoiledRanges[indexCompare].first
                    && nonSpoiledRanges[index].second >= nonSpoiledRanges[indexCompare].first)
                {
                    nonSpoiledRanges[index].second = std::max(nonSpoiledRanges[indexCompare].second, nonSpoiledRanges[index].second);
                    nonSpoiledRanges.erase(nonSpoiledRanges.begin() + indexCompare);
                    numMoves++;
                }
            }
        }
    }
    while (numMoves != 0);
    for (const auto& nonSpoiledRange : nonSpoiledRanges)
    {
       part2Count += nonSpoiledRange.second - nonSpoiledRange.first + 1;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Execution time: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "μs" << std::endl;
    std::printf("Result part 1: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET);
    std::printf("Result part 2: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET);
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    Day06("TestInput.txt");
    Day06("Input.txt");
    return 0;
}
