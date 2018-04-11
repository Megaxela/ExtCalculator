#pragma once

#include <stack>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <functional>
#include "ParsingException.hpp"
#include "StatementException.hpp"
#include "CalculationException.hpp"

/**
 * @brief Main calculator class.
 */
class Calculator
{
public:

    using NumberType = double;

    struct Function;

    struct Lexem
    {
        enum class Type
        {
            Unknown,
            Constant,
            Variable,
            Function,
            BraceOpen,
            BraceClosed,
            Comma
        };

        using ValueType = std::variant<
            Function*,
            std::size_t,
            NumberType
        >;

        Lexem() :
            type(Type::Unknown),
            value()
        {}

        Lexem(Lexem&& mv) noexcept :
            type(mv.type),
            value(std::move(mv.value))
        {

        }

        Lexem(const Lexem& mv) :
            type(mv.type),
            value(mv.value)
        {

        }

        explicit Lexem(Type type) :
            type(type),
            value()
        {

        }

        explicit Lexem(Type type, const ValueType& value) :
            type(type),
            value(value)
        {

        }

        Lexem& operator=(Lexem&& mv) noexcept
        {
            type = mv.type;
            value = std::move(mv.value);

            return *this;
        }

        Lexem& operator=(Lexem& mv)
        {
            type = mv.type;
            value = mv.value;

            return *this;
        }

        Type type;
        ValueType value;
    };

    using LexemStack = std::deque<Lexem>;
    using ArgumentsStack = std::vector<NumberType>;

    struct Function
    {
        Function() :
            name(),
            numberOfArguments(0),
            priority(0),
            function()
        {

        }

        Function(std::string name,
                 uint32_t numberOfArguments,
                 std::size_t priority,
                 double (*function)(ArgumentsStack&)
        ) :
            name(std::move(name)),
            numberOfArguments(numberOfArguments),
            priority(priority),
            function(function)
        {

        }

        Function(Function&& mv) noexcept :
            name(std::move(mv.name)),
            numberOfArguments(mv.numberOfArguments),
            priority(mv.priority),
            function(mv.function)
        {

        }

        Function& operator=(Function&& mv) noexcept
        {
            name = std::move(mv.name);
            numberOfArguments = mv.numberOfArguments;
            priority = mv.priority;
            function = mv.function;

            return *this;
        }

        Function& operator=(const Function& mv)
        {
            name = mv.name;
            numberOfArguments = mv.numberOfArguments;
            priority = mv.priority;
            function = mv.function;

            return (*this);
        }

        std::string name;
        uint32_t numberOfArguments;
        std::size_t priority;
        double (*function)(ArgumentsStack&);
    };

    /**
     * @brief Constructor.
     */
    Calculator();

    /**
     * @brief Method for setting exception
     * This method will perform lexical parsing.
     * @param expression Expression.
     */
    void setExpression(std::string expression, bool optimize=true);

    /**
     * @brief Method for adding new function to
     * calculator.
     * @param name Function name.
     * @param func Function definition.
     */
    void addFunction(
        Function func
    );

    /**
     * @brief Method for executing expression.
     * Can throw CalculationException on any error.
     * @return Execution result.
     */
    double execute();

    /**
     * @brief Method for adding basic functions.
     *
     * Functions list:
     * `+` - add
     * `-` - substract
     * `*` - multiply
     * `/` - divide
     * `^` - power
     * `!` - factorial
     * `&` - binary and
     * `|` - binary or
     * `%` - module
     * `abs` - absolute value
     * `sin` - sinus
     * `cos` - cosinus
     */
    void addBasicFunctions();

    /**
     * @brief Method for setting variable value.
     * @param name Variable name.
     * @param value Variable value.
     */
    void setVariable(std::string name, double value);

    /**
     * @brief Method for deleting variable name.
     * std::invalid_argument exception will
     * be thrown if there is no variable with this name.
     * @param name Name.
     */
    void deleteVariable(const std::string& name);

    /**
     * @brief Method for getting parsed RPN.
     * @param lexems Lexems.
     */
    void getRPN(LexemStack& lexems);

private:
    enum SymbolType
    {
        Alphabetic,
        Special,
        Decimal,
        Braces,
        Garbage
    };

    void splitOnLexems(std::string string, LexemStack& lexems);

    Calculator::SymbolType getSymbolType(char c);

    // Splitting on lexems states
    void noneState  (char*& string, LexemStack& lexems, int& state);
    void numberState(char*& string, LexemStack& lexems, int& state);
    void stringState(char*& string, LexemStack& lexems, int& state);
    void braceState (char*& string, LexemStack& lexems, int& state);

    // Validating pushed values
    void performValidation();

    // Pushing lexems as RPN
    void pushLexems(LexemStack& lexems);

    // Optimization
    void performOptimization();

    std::map<std::size_t, Function> m_functions;
    LexemStack m_expression;
    std::map<std::size_t, double> m_variables;

    int m_braceTest;

    ArgumentsStack m_executionStack;

    std::hash<std::string_view> m_stringHash;
};

std::ostream& operator<<(std::ostream& stream, const Calculator::LexemStack& stack);