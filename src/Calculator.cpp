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
    m_variables(),
    m_constants(),
    m_expression(),
    m_braceTest(0),
    m_executionStack(),
    m_stringHash()
{
    addFunction(
        Calculator::Function(
            "+",
            2, // Binary function. Undefined number of args
            1,
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::tgamma(value + 1);
            }
        )
    );
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

void Calculator::noneState(char*& string, LexemStack& /* lexems */, int& state)
{
    if (isdigit(*string) ||
        *string == '.' ||
        *string == '-' ||
        *string == '+')
    {
        state = 1; // Number
        return;
    }
    else if (*string == '(' ||
             *string == ')' ||
             *string == ']' ||
             *string == '[' ||
             *string == '{' ||
             *string == '}')
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

void Calculator::numberState(char*& string, LexemStack& lexems, int& state)
{
    auto start = string;
    auto dotFound = false;

    while (*string)
    {
        if (*string == '.')
        {
            if (dotFound)
            {
                throw ParsingException("NumberType dot detected in number");
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

            NumberType value;

            try
            {
                auto terminator_bu = *string;
                *string = '\0';

                value = std::atof(start);

                *string = terminator_bu;
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

        NumberType value;

        try
        {
            auto terminator_bu = *string;
            *string = '\0';

            value = std::atof(start);

            *string = terminator_bu;
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
    for (char i : {'(', ')',
                   '[', ']',
                   '{', '}'})
    {
        if (i == c)
        {
            return SymbolType::Braces;
        }
    }

    if (std::ispunct(c))
    {
        return SymbolType::Special;
    }

    if (std::isalpha(c))
    {
        return SymbolType::Alphabetic;
    }

    if (std::isdigit(c))
    {
        return SymbolType::Decimal;
    }

    return SymbolType::Garbage;
}

void Calculator::stringState(char*& string, LexemStack& lexems, int& state)
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

    SymbolType newSymbolType = symbolType;
    while (*string &&
           (symbolType == newSymbolType ||
               (symbolType == SymbolType::Alphabetic &&
                newSymbolType == SymbolType::Decimal) ||
               (symbolType == SymbolType::Alphabetic &&
                newSymbolType == SymbolType::Special &&
                *string == '_')))
    {
        ++string;

        if (*string)
        {
            newSymbolType = getSymbolType(*string);
        }
    }

    auto size = std::distance(start, string);

    // If there is some strange error.
    if (size == 0)
    {
        throw ParsingException("Internal error");
    }

    auto nameHash = m_stringHash(std::string_view(start, static_cast<std::string::size_type>(size)));

    auto function = m_functions.find(nameHash);

    if (function != m_functions.end())
    {
        lexem.value = &function->second;
        lexem.type = Lexem::Type::Function;

        lexems.emplace_back(std::move(lexem));
        state = 0; // None state
        return;
    }

    auto constant = m_constants.find(nameHash);

    if (constant != m_constants.end())
    {
        lexem.value = constant->second;
        lexem.type = Lexem::Type::Constant;

        lexems.emplace_back(std::move(lexem));
        state = 0;
        return;
    }

    // It's variable then
    lexem.value = nameHash;
    lexem.type = Lexem::Type::Variable;

    lexems.emplace_back(std::move(lexem));

    state = 0; // None state
}

void Calculator::braceState(char*& string, LexemStack& lexems, int& state)
{
    if (*string == '(' ||
        *string == '[' ||
        *string == '{')
    {
        lexems.emplace_back(
            Calculator::Lexem(
                Calculator::Lexem::Type::BraceOpen
            )
        );


        switch (*string)
        {
        case '(':
            m_braceTest += 1;
            break;
        case '[':
            m_braceTest += 2;
            break;
        case '{':
            m_braceTest += 3;
            break;
        }
        ++string;
    }
    else if (*string == ')' ||
             *string == ']' ||
             *string == '}')
    {
        lexems.emplace_back(
                Calculator::Lexem(
                    Calculator::Lexem::Type::BraceClosed
                )
        );

        switch (*string)
        {
        case ')':
            m_braceTest -= 1;
            break;
        case ']':
            m_braceTest -= 2;
            break;
        case '}':
            m_braceTest -= 3;
            break;
        }
        ++string;
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
    using LexemParserState = void(Calculator::*)(char*&, LexemStack&, int&);

    // 0 - none state.
    // 1 - parsing number
    // 2 - parsing string (function/variable)
    // 3 - quote

    static LexemParserState states[] = {
        &Calculator::noneState,
        &Calculator::numberState,
        &Calculator::stringState,
        &Calculator::braceState,
    };

    auto state = 0;

    auto s = string.c_str();

    m_braceTest = 0;
    while (*s)
    {
        int newState = -1;

        ((*this).*states[state])(const_cast<char*&>(s), lexems, newState);

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
    m_functions[m_stringHash(func.name)] = std::move(func);
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

void Calculator::setVariable(std::string name, NumberType value)
{
    m_variables[m_stringHash(name)] = value;
}

void Calculator::deleteVariable(const std::string& name)
{
    auto search_result = m_variables.find(m_stringHash(name));

    if (search_result == m_variables.end())
    {
        throw std::invalid_argument(
            std::string("There is no variable \"")
                .append(name.data(), name.size())
                .append("\"")
        );
    }

    m_variables.erase(search_result);
}

void Calculator::performOptimization()
{
    LexemStack stack;

    m_executionStack.clear();

    uint32_t args = 0;
    Function* func = nullptr;

    for (auto&& lexem : m_expression)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Constant:
            m_executionStack.push_back(std::get<NumberType>(lexem.value));
            break;

        case Lexem::Type::Variable:
            for (auto&& el : m_executionStack)
            {
                stack.emplace_back(
                    Lexem(
                        Lexem::Type::Constant,
                        el
                    )
                );
            }

            m_executionStack.clear();

            stack.push_back(lexem);

            break;

        case Lexem::Type::Function:
            func = std::get<Function*>(lexem.value);
            args = func->numberOfArguments;

            if (m_executionStack.size() >= args)
            {
                m_executionStack.emplace_back(
                    func->function(m_executionStack)
                );
            }
            else
            {
                for (auto&& el : m_executionStack)
                {
                    stack.emplace_back(
                        Lexem(
                            Lexem::Type::Constant,
                            el
                        )
                    );
                }

                m_executionStack.clear();

                stack.push_back(lexem);
            }

            break;

        default:
            break;
        }
    }
    
    for (auto&& el : m_executionStack)
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

Calculator::NumberType Calculator::execute()
{
    auto variableValue = m_variables.end();

    m_executionStack.clear();

    for (auto&& lexem : m_expression)
    {
        switch (lexem.type)
        {
        case Lexem::Type::Constant:
            m_executionStack.push_back(std::get<NumberType>(lexem.value));
            break;
        case Lexem::Type::Variable:
            variableValue = m_variables.find(std::get<std::size_t>(lexem.value));

            if (variableValue == m_variables.end())
            {
                throw CalculationException(
                    std::string("No variable \"")
                        .append(std::to_string(std::get<std::size_t>(lexem.value)))
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
            "abs",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::abs(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "sin",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::cos(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "tan",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::tan(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "acos",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::acos(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "asin",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::asin(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "tan",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::tan(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "atan2",
            2, // Two args
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
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
            "cosh",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::cosh(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "sinh",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::sinh(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "tanh",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::tanh(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "log",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::log(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "log10",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::log10(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "sqrt",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::sqrt(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "ceil",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::ceil(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "floor",
            1, // One arg
            4,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value = stack.back();
                stack.pop_back();

                return std::floor(value);
            }
        )
    );

    addFunction(
        Calculator::Function(
            "%",
            2, // Two args
            2,
            [](Calculator::ArgumentsStack& stack) -> NumberType
            {
                auto value2 = stack.back();
                stack.pop_back();

                auto value1 = stack.back();
                stack.pop_back();

                return std::fmod(value1, value2);
            }
        )
    );
}

void Calculator::addLogicFunctions()
{
    addFunction(
        Calculator::Function(
            ">",
            2,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto rhs = arguments.back();
                arguments.pop_back();

                auto lhs = arguments.back();
                arguments.pop_back();

                return lhs > rhs;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "<",
            2,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto rhs = arguments.back();
                arguments.pop_back();

                auto lhs = arguments.back();
                arguments.pop_back();

                return lhs < rhs;
            }
        )
    );

    addFunction(
        Calculator::Function(
            ">=",
            2,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto rhs = arguments.back();
                arguments.pop_back();

                auto lhs = arguments.back();
                arguments.pop_back();

                return lhs >= rhs;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "<=",
            2,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto rhs = arguments.back();
                arguments.pop_back();

                auto lhs = arguments.back();
                arguments.pop_back();

                return lhs <= rhs;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "==",
            2,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto rhs = arguments.back();
                arguments.pop_back();

                auto lhs = arguments.back();
                arguments.pop_back();

                return lhs == rhs;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "!=",
            2,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto rhs = arguments.back();
                arguments.pop_back();

                auto lhs = arguments.back();
                arguments.pop_back();

                return lhs != rhs;
            }
        )
    );

    addFunction(
        Calculator::Function(
            "if",
            3,
            0,
            [](Calculator::ArgumentsStack& arguments) -> NumberType
            {
                auto second = arguments.back();
                arguments.pop_back();

                auto first = arguments.back();
                arguments.pop_back();

                auto expression = arguments.back();
                arguments.pop_back();

                return expression ? first : second;
            }
        )
    );
}

void Calculator::addConstants()
{
    addConstant("Pi", M_PI);
    addConstant("e",  M_E);
}

void Calculator::addConstant(std::string name, NumberType value)
{
    m_constants[m_stringHash(name)] = value;
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
            stream << std::get<Calculator::NumberType>(lexem.value);
            break;
        case Calculator::Lexem::Type::Variable:
            stream << 'V' << std::get<std::size_t>(lexem.value);
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
