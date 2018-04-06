#include <iostream>
#include "Calculator.hpp"

void printLexems(const std::vector<Calculator::Lexem>& lexems)
{
    for (auto&& lexem : lexems)
    {
        switch (lexem.type)
        {
        case Calculator::Lexem::Type::Unknown:
            break;
        case Calculator::Lexem::Type::Constant:
            std::cout << std::get<double>(lexem.value);
            break;
        case Calculator::Lexem::Type::Variable:
            std::cout << std::get<std::string_view>(lexem.value);
            break;
        case Calculator::Lexem::Type::Function:
            std::cout << std::get<Calculator::Function*>(lexem.value)->name;
            break;
        case Calculator::Lexem::Type::BraceOpen:
            std::cout << '(';
            break;
        case Calculator::Lexem::Type::BraceClosed:
            std::cout << ')';
            break;
        }
        std::cout << ' ';
    }

    std::cout << std::endl;
}

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

    std::vector<Calculator::Lexem> lexems;
    calc.getRPN(lexems);

    printLexems(lexems);

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

int execute_debug(int argc, char** argv)
{
    if (argc < 1)
    {
        std::cerr << "Need expression to convert." << std::endl;
        return 1;
    }

    Calculator calc;
    calc.addBasicFunctions();

    calc.setExpression(argv[0]);


}

static std::map<std::string_view, std::function<int(int, char**)>> functions = {
    {"get_rpn", &rpn},
    {"execute", &execute}
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