#pragma once

#include <stack>
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <functional>

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
            BraceClosed
        };

        using ValueType = std::variant<
            Function*,
            std::string_view,
            NumberType
        >;

        Lexem() :
            type(Type::Unknown),
            value()
        {}

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

        Type type;
        ValueType value;
    };

    using LexemStack = std::stack<Lexem>;

    struct Function
    {
        enum class Type
        {
            Unknown,
            Unary,
            Binary,
            Function
        };

        Function() :
            name(),
            type(Type::Unknown),
            priority(0),
            function()
        {

        }

        Function(std::string_view name,
                 Type type,
                 std::size_t priority,
                 std::function<double(LexemStack&)> function) :
            name(name),
            type(type),
            priority(priority),
            function(std::move(function))
        {

        }

        std::string_view name;
        Type type;
        std::size_t priority;
        std::function<double(LexemStack&)> function;
    };

    /**
     * @brief Method for setting exception
     * This method will perform lexical parsing.
     * @param expression Expression.
     */
    void setExpression(const std::string& expression);

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
    void setVariable(std::string_view name, double value);

    /**
     * @brief Method for deleting variable name.
     * std::invalid_argument exception will
     * be thrown if there is no variable with this name.
     * @param name Name.
     */
    void deleteVariable(std::string_view name);

    /**
     * @brief Method for getting parsed RPN.
     * @param lexems Lexems.
     */
    void getRPN(std::vector<Lexem>& lexems);

private:
    enum SymbolType
    {
        Alphabetic,
        Special,
        Decimal,
        Braces,
        Garbage
    };

    void splitOnLexems(const std::string& string, std::vector<Lexem>& lexems);

    Calculator::SymbolType getSymbolType(char c);

    // Splitting on lexems states
    void noneState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state);
    void numberState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state);
    void stringState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state);
    void quoteState(const char*& string, std::vector<Calculator::Lexem>& lexems, int& state);

    // Validating initial splitting on lexems and statement
    void performValidation(const std::vector<Lexem>& lexems);

    // Pushing lexems as RPN
    void pushLexems(std::vector<Lexem>& lexems);

    std::map<std::string_view, Function> m_functions;
    std::vector<Lexem> m_expression;
    std::map<std::string_view, double> m_variables;
};

