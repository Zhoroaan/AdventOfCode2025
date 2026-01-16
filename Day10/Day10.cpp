#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <Eigen/Dense>


const char* ANSI_BOLD_UNDERLINE ="\033[1m\033[4m";
const char* ANSI_RESET = "\033[0m";

struct Machine
{
    std::string Lights;
    std::vector<std::vector<int32_t>> Buttons;
    std::vector<int32_t> JOltageRequirements;
};

struct Part1Leaf
{
    Part1Leaf(const std::string& InLights, int InDepth, const std::string& InParent)
        : Lights(InLights), Parent(InParent), Depth(InDepth) 
    {}
    std::string Lights;
    std::string Parent;
    int Depth = -1;
};

struct Part2Leaf
{
    Part2Leaf(const std::vector<int32_t> InEnergyLevels, int InDepth, const std::vector<int32_t>& InParent)
        : EnergyLevels(InEnergyLevels), Parent(InParent), Depth(InDepth) 
    {}
    std::vector<int32_t> EnergyLevels;
    std::vector<int32_t> Parent;
    int Depth = -1;
};

std::string CalculateNewLight(const std::string& InString, const std::vector<int32_t>& InButtonCombo)
{
    std::string returnString = InString;
    for (int32_t buttonCombo : InButtonCombo)
    {
        if (buttonCombo >= 0 && buttonCombo < InString.size())
        {
            returnString[buttonCombo] = returnString[buttonCombo] == '.' ? '#' : '.';
        }
    }
    return returnString;
}

std::vector<int32_t> CalculateNewEnergy(const std::vector<int32_t>& InCombo, const std::vector<int32_t>& InButtonCombo)
{
    std::vector<int32_t> newCombo = InCombo;
    for (int32_t buttonCombo : InButtonCombo)
    {
        if (buttonCombo >= 0 && buttonCombo < InCombo.size())
        {
            newCombo[buttonCombo]++;
        }
    }
    return newCombo;
}

int64_t FindSmallestButtonCombo(const std::vector<Machine>::value_type& machine)
{
    std::map<std::string, Part1Leaf> leaves;
    std::string input(machine.Lights.size(), '.');
    leaves.insert({input, Part1Leaf(input, 0, "")});
    for (int depth = 0; depth < 1000; ++depth)
    {
        for (const auto& parentLeaf : leaves)
        {
            if (parentLeaf.second.Depth != depth)
                continue;
            for (const std::vector<int32_t>& buttonCombo : machine.Buttons)
            {
                std::string newLights = CalculateNewLight(parentLeaf.second.Lights, buttonCombo);
                if (newLights == machine.Lights)
                {
                    return depth + 1;
                }
                if (!leaves.contains(newLights))
                {
                    leaves.insert({newLights, Part1Leaf(newLights, depth + 1, parentLeaf.second.Parent)});
                }
            }
        }
    }
    return 0;
}

int64_t FindSmallestButtonComboPart2(const std::vector<Machine>::value_type& machine)
{
    std::vector<int32_t> input(machine.JOltageRequirements.size(), 0);
    Eigen::MatrixXd A(6,5);
    
    return 0;
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
        Machine newMachine;
        std::getline(inputFile, inputLine);
        std::regex e(R"(\[([^\]]*)\])");
        
        std::smatch match;
        if (std::regex_search(inputLine, match, e))
        {
            newMachine.Lights = match[1];
        }
        
        std::regex e2(R"(\([\d,]+\))");
        std::sregex_iterator iter2(inputLine.begin(), inputLine.end(), e2);
        std::sregex_iterator end;
        if (iter2 == end)
            continue;
        for (std::sregex_iterator subitr = iter2; subitr != end; ++subitr)
        {
            std::regex e3(R"(\d+)");
            std::string searchString = subitr->str();
            std::sregex_iterator iter3(searchString.begin(), searchString.end(), e3);
            if (iter3 == end)
                continue;
            
            std::vector<int32_t> buttonActions;
            for (std::sregex_iterator subitr2 = iter3; subitr2 != end; ++subitr2)
            {
                std::string valueString = subitr2->str();
                if (!valueString.empty())
                    buttonActions.emplace_back(std::atoi(subitr2->str().c_str()));
            }
            if (!buttonActions.empty())
                newMachine.Buttons.emplace_back(buttonActions);
        }
        std::regex e4(R"(\{([\d,]+)\})");
        std::sregex_iterator iter4(inputLine.begin(), inputLine.end(), e4);
        if (std::regex_search(inputLine, match, e4)) {
            std::string inside = match[1]; // "3,5,4,7"
            std::stringstream ss(inside);
            std::string token;

            while (std::getline(ss, token, ',')) {
                newMachine.JOltageRequirements.emplace_back(std::stoi(token));
            }
        }
        machines.emplace_back(newMachine);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    part1Count = 0;
    for (const auto& machine : machines)
    {
        part1Count += FindSmallestButtonCombo(machine);
        part2Count += FindSmallestButtonComboPart2(machine);
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
    Day12("TestInput.txt", 7, 33);
    Day12("Input.txt", 535,1572047142);
    return 0;
}
