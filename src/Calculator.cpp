#include <system_error>
#include <functional>
#include <ParsingException.hpp>
#include <iostream>
#include <StatementException.hpp>
#include <cmath>
#include <CalculationException.hpp>
#include "Calculator.hpp"

Calculator::Calculator() :
    m_functions(),
    m_expression(),
    m_variables(),
    m_braceTest(0),
    m_executionStack()
{

}

void Calculator::setExpression(std::string expression, bool optimize)
{
    m_expression.clear();

    // Splitting on lexems
    LexemStack lexems;

    splitOnLexems(std::move(expression), lexems);

    pushLexems(lexems);

    performValidation();

    if (optimize)
    {
        performOptimization();
    }
}

void Calculator::getRPN(LexemStack& lexems)
{
    lexems = m_expression;
}

void Calculator::performValidation()
{
    uint32_t values = 0;

    Function* func;

    for (auto&& lexem : m_expression)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Constant:
        case Lexem::Type::Variable:
            values += 1;
            break;

        case Lexem::Type::Function:

            func = std::get<Function*>(lexem.value);

            if (func->numberOfArguments > values)
            {
                throw StatementException(
                    std::string("Not enough arguments for function \"")
                        .append(func->name)
                        .append("\"")
                );
            }

            values -= func->numberOfArguments - 1;

            break;
        default:
            break;
        }
    }

    if (values != 1)
    {
        throw StatementException("Unbalanced statement");
    }
}

void Calculator::noneState(const char*& string, LexemStack& /* lexems */, int& state)
{
    if (isdigit(*string) ||
        *string == '.' ||
        *string == '-' ||
        *string == '+')
    {
        state = 1; // Number
        return;
    }
    else if (*string == '(' || *string == ')')
    {
        state = 3; // Braces
        return;
    }
    else if (*string != ' ')
    {
        state = 2; // Var or func (string)
        return;
    }

    ++string;
}

void Calculator::numberState(const char*& string, LexemStack& lexems, int& state)
{
    auto start = string;
    auto dotFound = false;

    while (*string)
    {
        if (*string == '.')
        {
            if (dotFound)
            {
                throw ParsingException("Double dot detected in number");
            }

            dotFound = true;
            ++string;
        }
        else if (std::isdigit(*string) ||
                 (std::distance(start, string) < 1 &&
                  (*string == '-' ||
                   *string == '+')))
        {
            ++string;
        }
        else
        {
            // Finish
            auto len = std::distance(start, string);

            if (((*start == '-' ||
                  *start == '+') &&
                 (len < 2)) ||
                (!lexems.empty() &&
                 lexems.back().type != Lexem::Type::BraceOpen &&
                 lexems.back().type != Lexem::Type::Function &&
                 lexems.back().type != Lexem::Type::Comma))
            {
                string = start;
                state = 2; // String/Function/Variable
                return;
            }

            double value;

            try
            {
                // Optimize
                value = std::stod(std::string(start, len));
            }
            catch (std::invalid_argument& argument)
            {
                throw ParsingException(std::string(start, len) + " is not a number");
            }

            lexems.emplace_back(
                Calculator::Lexem(
                    Calculator::Lexem::Type::Constant,
                    value
                )
            );

            state = 0; // None state

            return;
        }
    }

    if (*string == '\0')
    {
        // Finish
        auto len = std::distance(start, string);

        if (((*start == '-' ||
              *start == '+') &&
             (len < 2)) ||
            (!lexems.empty() &&
             lexems.back().type != Lexem::Type::BraceOpen &&
             lexems.back().type != Lexem::Type::Function &&
             lexems.back().type != Lexem::Type::Comma))
        {
            string = start;
            state = 2; // String/Function/Variable
            return;
        }

        double value;

        try
        {
            // Optimize
            value = std::stod(std::string(start, len));
        }
        catch (std::invalid_argument& argument)
        {
            throw ParsingException(std::string(start, len) + " is not a number");
        }

        lexems.emplace_back(
            Calculator::Lexem(
                Calculator::Lexem::Type::Constant,
                value
            )
        );
    }
}

Calculator::SymbolType Calculator::getSymbolType(char c)
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

void Calculator::stringState(const char*& string, LexemStack& lexems, int& state)
{
    Lexem lexem;

    auto start = string;

    // Detecting symbol type
    SymbolType symbolType = getSymbolType(*string);

    if (symbolType == SymbolType::Garbage)
    {
        throw ParsingException("Garbage in variable/function name");
    }

    if (symbolType == SymbolType::Decimal)
    {
        throw ParsingException("Variable or function can't start with number");
    }

    if (symbolType == SymbolType::Special &&
        *string == ',')
    {
        lexem.type = Lexem::Type::Comma;

        lexems.emplace_back(std::move(lexem));

        state = 0;

        ++string;

        return;
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

    std::string name(start, static_cast<std::string::size_type>(size));

    bool found = false;

    auto function = m_functions.find(name);

    if (function != m_functions.end())
    {
        lexem.value = &function->second;
        lexem.type = Lexem::Type::Function;

        found = true;
    }

    if (found)
    {
        lexems.emplace_back(std::move(lexem));
        state = 0; // None state
        return;
    }

    // It's variable then
    lexem.value = std::move(name);
    lexem.type = Lexem::Type::Variable;

    lexems.emplace_back(std::move(lexem));

    state = 0; // None state
}

void Calculator::braceState(const char*& string, LexemStack& lexems, int& state)
{
    if (*string == '(')
    {
        lexems.emplace_back(Calculator::Lexem(Calculator::Lexem::Type::BraceOpen));
        ++string;

        ++m_braceTest;
    }
    else if (*string == ')')
    {
        lexems.emplace_back(Calculator::Lexem(Calculator::Lexem::Type::BraceClosed));
        ++string;

        --m_braceTest;
    }
    else
    {
        throw ParsingException(
            std::string("Unknown brace \"")
                .append(string, 1)
                .append("\" found")
        );
    }

    state = 0;
}

void Calculator::splitOnLexems(std::string string, LexemStack& lexems)
{
    using LexemParserState = std::function<void(const char*&, LexemStack&, int&)>;

    // 0 - none state.
    // 1 - parsing number
    // 2 - parsing string (function/variable)
    // 3 - quote

    LexemParserState states[] = {
        std::bind(&Calculator::noneState,   this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3),
        std::bind(&Calculator::numberState, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3),
        std::bind(&Calculator::stringState, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3),
        std::bind(&Calculator::braceState,  this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3)
    };

    auto state = 0;

    auto s = string.c_str();

    m_braceTest = 0;
    while (*s)
    {
        int newState = -1;

        states[state](s, lexems, newState);

        if (newState > -1)
        {
            state = newState;
        }
    }

    if (m_braceTest)
    {
        throw StatementException("Unbalanced braces");
    }
}

void Calculator::addFunction(Calculator::Function func)
{
    m_functions[func.name] = std::move(func);
}

void Calculator::pushLexems(LexemStack& lexems)
{
    m_expression.clear();

    LexemStack stack;

    for (auto&& lexem : lexems)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Unknown:
            break;
        case Lexem::Type::Constant: // passthrough
        case Lexem::Type::Variable:
            m_expression.emplace_back(std::move(lexem));
            break;
        case Lexem::Type::Function:
            if (!stack.empty() &&
                stack.back().type != Lexem::Type::BraceOpen &&
                std::get<Function*>(lexem.value)->priority <= std::get<Function*>(stack.back().value)->priority)
            {
                m_expression.emplace_back(std::move(stack.back()));
                stack.pop_back();
            }

            stack.emplace_back(std::move(lexem));

            break;
        case Lexem::Type::BraceOpen:
            stack.push_back(std::move(lexem));
            break;
        case Lexem::Type::BraceClosed:
            while (stack.back().type != Lexem::Type::BraceOpen)
            {
                m_expression.emplace_back(std::move(stack.back()));
                stack.pop_back();
            }

            stack.pop_back();

            break;

        default:
            break;
        }
    }

    while (!stack.empty())
    {
        m_expression.emplace_back(std::move(stack.back()));
        stack.pop_back();
    }
}

void Calculator::setVariable(std::string name, double value)
{
    m_variables[std::move(name)] = value;
}

void Calculator::deleteVariable(const std::string& name)
{
    auto search_result = m_variables.find(name);

    if (search_result == m_variables.end())
    {
        throw std::invalid_argument(
            std::string("There is no variable \"")
                .append(name.data(), name.size())
                .append("\"")
        );
    }

    m_variables.erase(name);
}

void Calculator::performOptimization()
{
    LexemStack stack;
    ArgumentsStack argStack;

    uint32_t args = 0;
    Function* func = nullptr;

    for (auto&& lexem : m_expression)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Constant:
            argStack.push_back(std::get<double>(lexem.value));
            break;

        case Lexem::Type::Variable:
            for (auto&& el : argStack)
            {
                stack.emplace_back(
                    Lexem(
                        Lexem::Type::Constant,
                        el
                    )
                );
            }

            argStack.clear();

            stack.push_back(lexem);

            break;

        case Lexem::Type::Function:
            func = std::get<Function*>(lexem.value);
            args = func->numberOfArguments;

            if (argStack.size() >= args)
            {
                argStack.emplace_back(
                    func->function(argStack)
                );
            }
            else
            {
                for (auto&& el : argStack)
                {
                    stack.emplace_back(
                        Lexem(
                            Lexem::Type::Constant,
                            el
                        )
                    );
                }

                argStack.clear();

                stack.push_back(lexem);
            }

            break;

        default:
            break;
        }
    }
    
    for (auto&& el : argStack)
    {
        stack.emplace_back(
            Lexem(
                Lexem::Type::Constant,
                el
            )
        );
    }

    m_expression = std::move(stack);
}

double Calculator::execute()
{
    auto variableValue = m_variables.end();

    m_executionStack.clear();

    for (auto&& lexem : m_expression)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Constant:
            m_executionStack.push_back(std::get<double>(lexem.value));
            break;
        case Lexem::Type::Variable:
            variableValue = m_variables.find(std::get<std::string>(lexem.value));

            if (variableValue == m_variables.end())
            {
                throw CalculationException(
                    std::string("No variable \"")
                        .append(std::get<std::string>(lexem.value))
                        .append("\" defined")
                );
            }

            m_executionStack.push_back(variableValue->second);

            break;

        case Lexem::Type::Function:
            m_executionStack.push_back(
                std::get<Function*>(lexem.value)->function(m_executionStack)
            );
            break;

        default:
            throw StatementException("Unexpected lexem detected");
        }
    }

    if (m_executionStack.size() != 1) // Result
    {
        throw StatementException("Unbalanced expression");
    }

    return m_executionStack.front();
}

void Calculator::addBasicFunctions()
{
    addFunction(
        Calculator::Function(
            "+",
            2, // Binary function. Undefined number of args
            1,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto rightValue = stack.back();
                stack.pop_back();

                auto leftValue = stack.back();
                stack.pop_back();

                return leftValue + rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "-",
            2, // Binary function. Undefined number of args
            1,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto rightValue = stack.back();
                stack.pop_back();

                auto leftValue = stack.back();
                stack.pop_back();

                return leftValue - rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "*",
            2, // Binary function. Undefined number of args
            2,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto rightValue = stack.back();
                stack.pop_back();

                auto leftValue = stack.back();
                stack.pop_back();

                return leftValue * rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "/",
            2, // Binary function. Undefined number of args
            2,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto rightValue = stack.back();
                stack.pop_back();

                auto leftValue = stack.back();
                stack.pop_back();

                return leftValue / rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "^",
            2, // Binary function. Undefined number of args
            3,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto rightValue = stack.back();
                stack.pop_back();

                auto leftValue = stack.back();
                stack.pop_back();

                return std::pow(leftValue, rightValue);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "!",
            1, // Binary function. Undefined number of args
            3,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto value = stack.back();
                stack.pop_back();

                return std::tgamma(value + 1);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "sin",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto value = stack.back();
                stack.pop_back();

                return std::sin(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "cos",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto value = stack.back();
                stack.pop_back();

                return std::cos(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "atan2",
            2, // Two args
            4,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto value2 = stack.back();
                stack.pop_back();

                auto value1 = stack.back();
                stack.pop_back();

                return std::atan2(value1, value2);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "exp",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> double
            {
                auto value = stack.back();
                stack.pop_back();

                return std::exp(value);
            }
        )
    );
}

std::ostream& operator<<(std::ostream& stream, const Calculator::LexemStack& stack)
{
    for (auto&& lexem : stack)
    {
        switch (lexem.type)
        {
        case Calculator::Lexem::Type::Unknown:
            stream << "???";
            break;
        case Calculator::Lexem::Type::Constant:
            stream << std::get<double>(lexem.value);
            break;
        case Calculator::Lexem::Type::Variable:
            stream << std::get<std::string>(lexem.value);
            break;
        case Calculator::Lexem::Type::Function:
            stream << std::get<Calculator::Function*>(lexem.value)->name;
            break;
        case Calculator::Lexem::Type::BraceOpen:
            stream << '(';
            break;
        case Calculator::Lexem::Type::BraceClosed:
            stream << ')';
            break;
        case Calculator::Lexem::Type::Comma:
            stream << ',';
            break;
        }

        std::cout << ' ';
    }

    return stream;
}
