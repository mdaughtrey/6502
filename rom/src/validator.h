#include <iostream>
#include <string>
#include <stdint.h>
#include <vector>
#include <regex>
#include "types.h"

class Validator
{
    public:
        std::string memo;
        std::string accumulator;
        std::string pattern;
        std::smatch matches;
        std::string prompt_string;

        Validator(std::string pattern_p = std::string(""), std::string prompt_p = std::string(""))
            : pattern(pattern_p), prompt_string(prompt_p)
        {
        }

        void accumulate(uint8_t input)
        {
            accumulator += input;
        }

        const char * expecting()
        {
            return pattern.c_str();
        }
        
        const std::string & prompt()
        {
            return prompt_string;
        }

        const std::string & accumulated()
        {
            return accumulator;
        }
        void clear()
        {
            accumulator.clear();
        }
        
        void deletelast()
        {
            accumulator.pop_back();
        }

        CommandInput validate(void)
        {
            std::regex re(pattern);
            CommandInput result;
//            printf("Validator: testing %s against %s\n", accumulator.c_str(), pattern.c_str());
            if (std::regex_search(accumulator, matches, re))
            {
                for (auto iter = matches.begin(); iter != matches.end(); iter++)
                {
                    result.push_back(iter->str());
                }
            }
            return result;
        }
};

// class NumericValidator : public Validator
// {
//     public:
//         bool validate(uint8_t input)
//         {
//             if (input >= '0' && input <= '9')
//             {
//                 accumulator += input;
//                 return true;
//             }
//             return false;
//         }
//         const char * expecting()
//         {
//             return "0-9";
//         }
// };
// 
// class HexValidator : public Validator
// {
//     public:
//         bool validate(uint8_t input)
//         {
//             if ((input >= '0' && input <= '9') || (input >= 'A' && input <= 'F') || (input >= 'a' && input <= 'f'))
//             {
//                 accumulator += input;
//                 return true;
//             }
//             return false;
//         }
// 
//         const char * expecting()
//         {
//             return "0-9A-Fa-f";
//         }
// };
// 
// class AddrLenValidator : public Validator
// {
//     public:
//         HexValidator hex_validator = HexValidator();
//         bool validate(uint8_t input)
//         {
//             if (accumulator.length() < 4 && hex_validator.validate(input))
//             {
//                 accumulator += input;
//                 return true;
//             }
//             else if (accumulator.length() == 4 && input == '/')
//             {
//                 accumulator += input;
//                 return true;
//             }
//             else if (accumulator.length() > 4 && hex_validator.validate(input))
//             {
//                 accumulator += input;
//                 return true;
//             }
//             return false;
//         }
// 
//         const char * expecting()
//         {
//             return "XXXX/XXXX";
//         }
// };
// 
// class PinStateValidator : public Validator
// {
//     public:
//         bool validate(uint8_t input)
//         {
//             if (accumulator.empty())
//             {
//                 if (input != 'i' && input != 'o')
//                 {
//                     return false;
//                 }
//             }
//             else if (accumulator.length() < 3)
//             {
//                 if (input < '0' ||  input > '9')
//                 {
//                     return false;
//                 }
//             }
//             else if (accumulator.length() < 4)
//             {
//                 if (input != '1' && input != '0')
//                 {
//                     return false;
//                 }
//             }
// 
//             accumulator += input;
//             printf("Accumulator: %s\r\n", accumulator.c_str());
//             return true;
//         }
//         const char * expecting()
//         {
//             return "[Dir/Pin[/Value] (i|o)NN(1|0)";
//         }
// };
