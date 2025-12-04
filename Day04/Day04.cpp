#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

//std::string Filename = "TestInput.txt";
std::string Filename = "Input.txt";

int32_t WrapTo99(int InCount) {
    return (InCount % 100 + 100) % 100;
}

int CountFreeRolls(const bool InbShouldRemoveFound, std::vector<std::vector<char>>& InOutBoard)
{
    int boundX = static_cast<int>(InOutBoard[0].size());
    int boundY = static_cast<int>(InOutBoard.size());
    int countFound = 0;
    for (int y = 0; y < InOutBoard.size(); ++y)
    {
        for (int x = 0; x < InOutBoard[y].size(); ++x)
        {
            if (InOutBoard[y][x] != '@')
                continue;
            int blockedCount = 0;
            for (int kernelX = x - 1; kernelX <= x + 1; ++kernelX)
            {
                for (int kernelY = y - 1; kernelY <= y + 1; ++kernelY)
                {
                    if (kernelX == x && kernelY == y)
                        continue;
                    if (kernelX < 0 || kernelX == boundX || kernelY < 0 || kernelY == boundY)
                        continue;
                    if (InOutBoard[kernelY][kernelX] == '@')
                        blockedCount++;
                }
            }
            if (blockedCount < 4)
            {
                countFound++;
                if (InbShouldRemoveFound)
                    InOutBoard[y][x] = '.';
            }
        }
    }
    return countFound;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    std::ifstream inputFile;
    inputFile.open(Filename);
    std::string inputLine;
    std::vector<std::vector<char>> board;

    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        board.emplace_back(inputLine.begin(), inputLine.end());
    }
    
    int64_t part1Count = CountFreeRolls(false, board);
    
    int64_t part2Count = 0;
    int64_t lastReturned;
    do
    {
        lastReturned = CountFreeRolls(true, board);
        part2Count += lastReturned;
    } while (lastReturned > 0);
    
    std::printf("Result part 1: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET);
    std::printf("Result part 2: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET);
    return 0;
}
