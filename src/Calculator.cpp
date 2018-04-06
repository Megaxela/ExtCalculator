#include <system_error>
#include <functional>
#include <ParsingException.hpp>
#include <iostream>
#include <StatementException.hpp>
#include <cmath>
#include <CalculationException.hpp>
#include "Calculator.hpp"

void Calculator::setExpression(const std::string& expression)
{
    m_expression.clear();

    // Splitting on lexems
    std::vector<Lexem> lexems;

    splitOnLexems(expression, lexems);
    performValidation(lexems);
    pushLexems(lexems);
}

void Calculator::getRPN(std::vector<Calculator::Lexem>& lexems)
{
    lexems = m_expression;
}

void Calculator::performValidation(const std::vector<Calculator::Lexem>& lexems)
{
    // Calculating braces
    int braces = 0;

    for (auto&& lexem : lexems)
    {
        switch (lexem.type)
        {
        case Lexem::Type::BraceOpen:
            ++braces;
            break;
        case Lexem::Type::BraceClosed:
            --braces;
            break;

        default:
            break;
        }
    }

    if (braces != 0)
    {
        throw StatementException("Braces are unbalanced");
    }
}

void Calculator::noneState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state)
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

void Calculator::numberState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state)
{
    auto start = string;
    auto dotFound = false;

    while (*string)
    {
        if (*string == '.')
        {
            if (dotFound)
            {
                throw ParsingException("Double dot detected in number.");
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
                 (len < 2)))
            {
                string = start;
                state = 2; // String
                return;
            }


            double value;

            try
            {
                value = std::stod(std::string(start, len));
            }
            catch (std::invalid_argument& argument)
            {
                throw ParsingException(std::string(start, len) + " is not a number.");
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

        double value;

        try
        {
            value = std::stod(std::string(start, len));
        }
        catch (std::invalid_argument& argument)
        {
            throw ParsingException(std::string(start, len) + " is not a number.");
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

void Calculator::stringState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state)
{
    auto start = string;

    // Detecting symbol type
    SymbolType symbolType = getSymbolType(*string);

    if (symbolType == SymbolType::Garbage)
    {
        throw ParsingException("Garbage in variable/function name");
    }

    while (*string &&
           (symbolType == getSymbolType(*string) ||
               (symbolType == SymbolType::Alphabetic &&
                getSymbolType(*string) == SymbolType::Decimal)))
    {
        ++string;
    }

    auto size = std::distance(start, string);

    // If there is some strange error.
    if (size == 0)
    {
        throw ParsingException("Internal error.");
    }

    std::string_view name(start, static_cast<std::string::size_type>(size));

    bool found = false;

    Lexem lexem;

    for (auto&& [key, value] : m_functions)
    {
        if (key == name)
        {
            lexem.value = &m_functions[key];
            lexem.type = Lexem::Type::Function;

            found = true;
            break;
        }
    }

    if (found)
    {
        lexems.emplace_back(lexem);
        state = 0; // None state
        return;
    }

    // It's variable then
    lexem.value = name;
    lexem.type = Lexem::Type::Variable;

    lexems.emplace_back(lexem);

    state = 0; // None state
}

void Calculator::quoteState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state)
{
    if (*string == '(')
    {
        lexems.emplace_back(Calculator::Lexem(Calculator::Lexem::Type::BraceOpen));
        ++string;
    }
    else if (*string == ')')
    {
        lexems.emplace_back(Calculator::Lexem(Calculator::Lexem::Type::BraceClosed));
        ++string;
    }
    else
    {
        throw ParsingException(std::string("Unknown brace ") + *string + " found.");
    }

    state = 0;
}

void Calculator::splitOnLexems(const std::string& string, std::vector<Calculator::Lexem>& lexems)
{
    using LexemParserState = std::function<void(const char*&, std::vector<Calculator::Lexem>&, int&)>;

    // 0 - none state.
    // 1 - parsing number
    // 2 - parsing string (function/variable)
    // 3 - quote

    LexemParserState states[] = {
        std::bind(&Calculator::noneState, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3),
        std::bind(&Calculator::numberState, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3),
        std::bind(&Calculator::stringState, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3),
        std::bind(&Calculator::quoteState, this, std::placeholders::_1,std::placeholders::_2, std::placeholders::_3)
    };

    auto state = 0;

    auto s = string.c_str();

    while (*s)
    {
        int newState = -1;

        states[state](s, lexems, newState);

        if (newState > -1)
        {
            state = newState;
        }
    }
}

void Calculator::addFunction(Calculator::Function func)
{
    m_functions[func.name] = std::move(func);
}

void Calculator::pushLexems(std::vector<Calculator::Lexem>& lexems)
{
    m_expression.clear();

    LexemStack stack;

    int step = 0;

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
                stack.top().type != Lexem::Type::BraceOpen &&
                std::get<Function*>(lexem.value)->priority <= std::get<Function*>(stack.top().value)->priority)
            {
                m_expression.emplace_back(std::move(stack.top()));
                stack.pop();
            }

            stack.emplace(std::move(lexem));

            break;
        case Lexem::Type::BraceOpen:
            stack.push(std::move(lexem));
            break;
        case Lexem::Type::BraceClosed:
            while (stack.top().type != Lexem::Type::BraceOpen)
            {
                m_expression.emplace_back(std::move(stack.top()));
                stack.pop();
            }

            stack.pop();

            break;
        }
    }

    while (!stack.empty())
    {
        m_expression.emplace_back(std::move(stack.top()));
        stack.pop();
    }
}

void Calculator::setVariable(std::string_view name, double value)
{
    m_variables[name] = value;
}

void Calculator::deleteVariable(std::string_view name)
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

double Calculator::execute()
{
    auto variableValue = m_variables.end();

    LexemStack stack;

    for (auto&& lexem : m_expression)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Constant:
            stack.push(lexem);
            break;
        case Lexem::Type::Variable:
            variableValue = m_variables.find(std::get<std::string_view>(lexem.value));

            if (variableValue == m_variables.end())
            {
                throw CalculationException(
                    std::string("No variable \"").append(
                        std::get<std::string_view>(lexem.value).data(),
                        std::get<std::string_view>(lexem.value).size()
                    ).append("\" defined.")
                );
            }

            stack.push(
                Lexem(
                    Lexem::Type::Constant,
                    variableValue->second
                )
            );

            break;

        case Lexem::Type::Function:
            stack.push(
                Lexem(
                    Lexem::Type::Constant,
                    std::get<Function*>(lexem.value)->function(stack)
                )
            );
            break;

        default:
            throw StatementException("Unexpected lexem detected.");
        }
    }

    if (stack.size() != 1) // Result
    {
        throw StatementException("Unbalanced expression.");
    }

    return std::get<double>(stack.top().value);
}

void Calculator::addBasicFunctions()
{
    addFunction(
        Calculator::Function(
            "+",
            Calculator::Function::Type::Binary,
            1,
            [](Calculator::LexemStack& stack) -> double
            {
                auto rightValue = std::get<double>(stack.top().value);
                stack.pop();

                if (stack.empty())
                {
                    return rightValue;
                }

                auto leftValue = std::get<double>(stack.top().value);
                stack.pop();

                return leftValue + rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "-",
            Calculator::Function::Type::Binary,
            1,
            [](Calculator::LexemStack& stack) -> double
            {
                auto rightValue = std::get<double>(stack.top().value);
                stack.pop();

                auto leftValue = std::get<double>(stack.top().value);
                stack.pop();

                return leftValue - rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "*",
            Calculator::Function::Type::Binary,
            2,
            [](Calculator::LexemStack& stack) -> double
            {
                auto rightValue = std::get<double>(stack.top().value);
                stack.pop();

                auto leftValue = std::get<double>(stack.top().value);
                stack.pop();

                return leftValue * rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "/",
            Calculator::Function::Type::Binary,
            2,
            [](Calculator::LexemStack& stack) -> double
            {
                auto rightValue = std::get<double>(stack.top().value);
                stack.pop();

                auto leftValue = std::get<double>(stack.top().value);
                stack.pop();

                return leftValue / rightValue;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "^",
            Calculator::Function::Type::Binary,
            3,
            [](Calculator::LexemStack& stack) -> double
            {
                auto rightValue = std::get<double>(stack.top().value);
                stack.pop();

                auto leftValue = std::get<double>(stack.top().value);
                stack.pop();

                return std::pow(leftValue, rightValue);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "sin",
            Calculator::Function::Type::Function,
            4,
            [](Calculator::LexemStack& stack) -> double
            {
                auto value = std::get<double>(stack.top().value);
                stack.pop();

                return std::sin(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "cos",
            Calculator::Function::Type::Function,
            4,
            [](Calculator::LexemStack& stack) -> double
            {
                auto value = std::get<double>(stack.top().value);
                stack.pop();

                return std::cos(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "atan2",
            Calculator::Function::Type::Function,
            4,
            [](Calculator::LexemStack& stack) -> double
            {
                auto value2 = std::get<double>(stack.top().value);
                stack.pop();

                auto value1 = std::get<double>(stack.top().value);
                stack.pop();

                return std::atan2(value1, value2);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "exp",
            Calculator::Function::Type::Function,
            4,
            [](Calculator::LexemStack& stack) -> double
            {
                auto value = std::get<double>(stack.top().value);
                stack.pop();

                return std::exp(value);
            }
        )
    );
}
