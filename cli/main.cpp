#include <iostream>
#include <ParsingException.hpp>
#include <StatementException.hpp>
#include <CalculationException.hpp>
#include "Calculator.hpp"

int rpn(int argc, char** argv)
{
    if (argc < 1)
    {
        std::cerr << "Need expression to convert." << std::endl;
        return 1;
    }

    Calculator calc;
    calc.addBasicFunctions();

    calc.setExpression(argv[0]);

    Calculator::LexemStack lexems;
    calc.getRPN(lexems);

    std::cout << lexems << std::endl;

    return 0;
}

int execute(int argc, char** argv)
{
    if (argc < 1)
    {
        std::cerr << "Need expression to convert." << std::endl;
        return 1;
    }

    Calculator calc;
    calc.addBasicFunctions();

    calc.setExpression(argv[0]);

    std::cout << calc.execute() << std::endl;

    return 0;
}

enum SymbolType
{
    Alphabetic,
    Special,
    Decimal,
    Braces,
    Garbage
};

SymbolType getSymbolType(char c)
{
    static char special[] = {
        '!',  '\"', '#', '$', '%', '&',  '\'', '*',
        '+',  ',',  '-', '.', '/', '\\', '^',  '_',
        '|',  '~'
    };

    static char braces[] = {
        '(', ')', '[', ']', '{', '}'
    };

    if (std::isalpha(c))
    {
        return SymbolType::Alphabetic;
    }

    if (std::isdigit(c))
    {
        return SymbolType::Decimal;
    }

    for (char i : special)
    {
        if (i == c)
        {
            return SymbolType::Special;
        }
    }

    for (char i : braces)
    {
        if (i == c)
        {
            return SymbolType::Braces;
        }
    }

    return SymbolType::Garbage;
}

std::string get_variable(const std::string& output)
{
    std::string result;

    auto string = output.c_str();

    // Skipping initial spaces
    while (*string == ' ')
    {
        ++string;
    }

    auto start = string;

    // Detecting symbol type
    auto symbolType = getSymbolType(*string);

    if (symbolType == SymbolType::Garbage)
    {
        throw ParsingException("Garbage in variable/function name");
    }

    if (symbolType == SymbolType::Decimal)
    {
        throw ParsingException("Variable or function can't start with number");
    }

    while (*string &&
           (symbolType == getSymbolType(*string) ||
            (symbolType == SymbolType::Alphabetic &&
             getSymbolType(*string) == SymbolType::Decimal) ||
            (symbolType == SymbolType::Alphabetic &&
             getSymbolType(*string) == SymbolType::Special &&
             *string == '_')))
    {
        ++string;
    }

    auto size = std::distance(start, string);

    // If there is some strange error.
    if (size == 0)
    {
        throw ParsingException("Internal error");
    }

    // Checking rest of string
    while (*string)
    {
        if (*string != ' ')
        {
            throw ParsingException("Wrong variable format");
        }

        ++string;
    }

    return std::string(start, static_cast<std::string::size_type>(size));
}

int interactive(int, char**)
{
    std::string output;

    std::cout << "Initializing calculator..." << std::endl;

    Calculator calculator;
    calculator.addBasicFunctions();

    std::cout << "Calculator initialized." << std::endl;

    std::string variableName;

    while (true)
    {
        std::cout << '>';
        std::getline(std::cin, output);

        if (output == "quit")
        {
            break;
        }

        try
        {
            // Trying to split function and variable declaration
            auto equalPos = output.find('=');
            if (equalPos != std::string::npos)
            {
                variableName = get_variable(output.substr(0, equalPos));

                std::cout << "Found variable \"" << variableName << "\"" << std::endl;

                if (variableName.empty())
                {
                    std::cout << "Variable can't be empty." << std::endl;
                    continue;
                }
            }

            calculator.setExpression(std::move(output.substr(equalPos + 1)));
        }
        catch (ParsingException& e)
        {
            std::cout << "Parsing exception: " << e.what() << std::endl << std::endl;
            continue;
        }
        catch (StatementException& e)
        {
            std::cout << "Statement exception: " << e.what() << std::endl << std::endl;
            continue;
        }

        try
        {
            auto result = calculator.execute();

            if (!variableName.empty())
            {
                std::cout << "Setting variable \"" << variableName << "\" with value " << result << std::endl;
                calculator.setVariable(std::move(variableName), result);

                std::exchange(variableName, "");
            }

            std::cout << result << std::endl << std::endl;
        }
        catch (CalculationException& e)
        {
            std::cout << "Calculation exception: " << e.what() << std::endl << std::endl;
        }
    }

    std::cout << "Calculator closed." << std::endl;

    return 0;
}

static std::map<std::string_view, std::function<int(int, char**)>> functions = {
    {"get_rpn", &rpn},
    {"execute", &execute},
    {"interactive", &interactive}
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Wrong usage. Synax:" << std::endl;

        if (argc > 0)
        {
            std::cout << argv[0] << " <command> <command_args>" << std::endl;
        }

        std::cout << "Command: " << std::endl;

        // Commands
        for (auto&& cmd : functions)
        {
            std::cout << "    " << cmd.first << std::endl;
        }

        return 1;
    }

    auto search_result = functions.find(argv[1]);

    if (search_result == functions.end())
    {
        std::cerr << "There is no function \"" << argv[1] << "\"." << std::endl;
        return 1;
    }

    return search_result->second(argc - 2, argv + 2);
}