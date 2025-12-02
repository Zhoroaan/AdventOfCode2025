#include <fstream>
#include <iostream>
#include <string>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

std::string Filename = "TestInput.txt";
//std::string Filename = "Input.txt";

int64_t Part1Check(int64_t InValue)
{
    std::string valueAsString = std::to_string(InValue);
    if (valueAsString.length() % 2 == 0)
    {
        std::string part1 = valueAsString.substr(0, valueAsString.length() >> 1);
        std::string part2 = valueAsString.substr(valueAsString.length() >> 1, valueAsString.length() >> 1);
        if (part1 == part2)
        {
             return InValue;
        }
    }
    return 0;
}

int64_t Part2Check(int64_t InValue)
{
    std::string valueAsString = std::to_string(InValue);
    for (size_t substringSize = 1; substringSize <= valueAsString.length() / 2; ++substringSize)
    {
        std::string firstSubstring;
        for (size_t startIndexToEvaluate = 0; startIndexToEvaluate < valueAsString.length(); startIndexToEvaluate += substringSize)
        {
            if (valueAsString.length() % substringSize != 0) // Valid only if evenly subdivided
                break;
            std::string subString = valueAsString.substr(startIndexToEvaluate, substringSize);
            if (firstSubstring.empty())
                firstSubstring = subString;
            else
            {
                if (subString != firstSubstring)
                    break;
                const bool isLastValue = startIndexToEvaluate + substringSize >= valueAsString.length();
                if (isLastValue)
                    return InValue;
            }
        }
    }
    return 0;
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    std::ifstream inputFile;
    inputFile.open(Filename);
    int64_t part1SumInvalidIds = 0;
    int64_t part2SumInvalidIds = 0;
    while (inputFile.is_open() && !inputFile.eof())
    {
        int64_t startRange, endRange;
        inputFile >> startRange;
        inputFile.ignore(1, '-');
        inputFile >> endRange;
        inputFile.ignore(1, ',');
        for (int64_t currentValue = startRange; currentValue <= endRange; currentValue++)
        {
            part1SumInvalidIds += Part1Check(currentValue);
            part2SumInvalidIds += Part2Check(currentValue);
        }
    }
    std::printf("Result part 1: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part1SumInvalidIds, ANSI_RESET);
    
    std::printf("Result part 2: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part2SumInvalidIds, ANSI_RESET);
    return 0;
}
