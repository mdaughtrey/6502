#include <string>
#include <stdint.h>

class Validator
{
    public:
        std::string accumulator;
        virtual bool validate(uint8_t input)
        {
            accumulator += input;
            return true;
        }

        virtual const char * expecting()
        {
            return "";
        }

        virtual std::string accumulated()
        {
            return accumulator;
        }
        void clear()
        {
            accumulator.clear();
        }
};

class NumericValidator : public Validator
{
    public:
        bool validate(uint8_t input)
        {
            if (input >= '0' && input <= '9')
            {
                accumulator += input;
                return true;
            }
            return false;
        }
        const char * expecting()
        {
            return "0-9";
        }
};

class HexValidator : public Validator
{
    public:
        bool validate(uint8_t input)
        {
            if ((input >= '0' && input <= '9') || (input >= 'A' && input <= 'F') || (input >= 'a' && input <= 'f'))
            {
                accumulator += input;
                return true;
            }
            return false;
        }

        const char * expecting()
        {
            return "0-9A-Fa-f";
        }
};

class AddrLenValidator : public Validator
{
    public:
        HexValidator hex_validator = HexValidator();
        bool validate(uint8_t input)
        {
            if (accumulator.length() < 4 && hex_validator.validate(input))
            {
                accumulator += input;
                return true;
            }
            else if (accumulator.length() == 4 && input == '/')
            {
                accumulator += input;
                return true;
            }
            else if (accumulator.length() > 4 && hex_validator.validate(input))
            {
                accumulator += input;
                return true;
            }
            return false;
        }

        const char * expecting()
        {
            return "XXXX/XXXX";
        }
};

class PinStateValidator : public Validator
{
    public:
        bool validate(uint8_t input)
        {
            if (accumulator.empty())
            {
                if (input != 'i' && input != 'o')
                {
                    return false;
                }
            }
            else if (accumulator.length() < 3)
            {
                if (input < '0' ||  input > '9')
                {
                    return false;
                }
            }
            else if (accumulator.length() < 4)
            {
                if (input != '1' && input != '0')
                {
                    return false;
                }
            }

            accumulator += input;
            printf("Accumulator: %s\r\n", accumulator.c_str());
            return true;
        }
        const char * expecting()
        {
            return "[Dir/Pin[/Value] (i|o)NN(1|0)";
        }
};
