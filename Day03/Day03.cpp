#include <array>
#include <fstream>
#include <iostream>
#include <string>

const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

//std::string Filename = "TestInput.txt";
std::string Filename = "Input.txt";

template <int T>
int64_t SearchBank(const std::string& InBank)
{
    std::array<char, T> resultDigits{};
    for (size_t bankIndex = 0; bankIndex < InBank.size(); bankIndex++)
    {
        for (size_t resultIndex = 0; resultIndex < resultDigits.size(); resultIndex++)
        {
            size_t requiredRemainingDigets = resultDigits.size() - resultIndex;
            if (requiredRemainingDigets <= InBank.size() - bankIndex && InBank[bankIndex] > resultDigits[resultIndex])
            {
                resultDigits[resultIndex] = InBank[bankIndex];
                for (size_t resetResultIndex = resultIndex + 1; resetResultIndex < resultDigits.size(); resetResultIndex++)
                    resultDigits[resetResultIndex] = 0;
                break;
            }
        }
    }
    return std::atoll(resultDigits.data());
}

int main(int /*InArgc*/, char* /*InArgv[]*/)
{
    std::ifstream inputFile;
    inputFile.open(Filename);
    std::string inputLine;
    int64_t part1Sum = 0;
    int64_t part2Sum = 0;
    while (inputFile.is_open() && !inputFile.eof())
    {
        std::getline(inputFile, inputLine);
        part1Sum += SearchBank<2>(inputLine);
        part2Sum += SearchBank<12>(inputLine);
    }
    std::printf("Result part 1, ends at 0: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part1Sum, ANSI_RESET);
    std::printf("Result part 2, Number of times past 0: %s%lld%s\n",
        ANSI_BOLD_UNDERLINE, part2Sum, ANSI_RESET);
    return 0;
}
