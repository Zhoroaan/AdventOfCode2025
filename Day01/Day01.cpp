#include <fstream>
#include <iostream>
#include <string>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

std::string Filename = "TestInput.txt";
//std::string Filename = "Input.txt";

int32_t WrapTo99(int InCount) {
    return (InCount % 100 + 100) % 100;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    std::ifstream inputFile;
    inputFile.open(Filename);
    std::string inputLine;
    
    int32_t currentNumber = 50;
    int32_t part1Count = 0;
    int32_t part2Count = 0;

    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        int32_t sign = inputLine[0] == 'L' ? -1 : 1;
        int32_t value = std::atoi(inputLine.c_str() + 1);
        
        part2Count += value / 100;
        int32_t wrappedDiff = (value * sign) % 100;
        const bool isOutsideOfRange = (currentNumber + wrappedDiff <= 0
            || currentNumber + wrappedDiff > 99);
        if (currentNumber != 0 && isOutsideOfRange)
            part2Count++;
        currentNumber = WrapTo99(currentNumber + sign * value);
        
        if (currentNumber == 0)
            part1Count++;
    }
    std::printf("Result part 1, ends at 0: %s%d%s\n",
        ANSI_BOLD_UNDERLINE, part1Count, ANSI_RESET);
    std::printf("Result part 2, Number of times past 0: %s%d%s\n",
        ANSI_BOLD_UNDERLINE, part2Count, ANSI_RESET);
    return 0;
}
