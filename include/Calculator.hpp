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
            Function*,   // Function
            std::size_t, // Variable hash
            NumberType   // Constant value
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

    /**
     * @brief Structure, that describes
     * calculator function.
     */
    struct Function
    {
        /**
         * @brief Default constructor.
         */
        Function() :
            name(),
            numberOfArguments(0),
            priority(0),
            function()
        {

        }

        /**
         * @brief Initializer constructor.
         * @param name Function name.
         * Will be used in parsing.
         * @param numberOfArguments Number of used arguments for
         * validation.
         * @param priority Function priority.
         * @param function Pointer to function.
         */
        Function(std::string name,
                 uint32_t numberOfArguments,
                 std::size_t priority,
                 NumberType (*function)(ArgumentsStack&)
        ) :
            name(std::move(name)),
            numberOfArguments(numberOfArguments),
            priority(priority),
            function(function)
        {

        }

        /**
         * @brief Move constructor.
         */
        Function(Function&& mv) noexcept :
            name(std::move(mv.name)),
            numberOfArguments(mv.numberOfArguments),
            priority(mv.priority),
            function(mv.function)
        {

        }

        /**
         * @brief Move operator.
         */
        Function& operator=(Function&& mv) noexcept
        {
            name = std::move(mv.name);
            numberOfArguments = mv.numberOfArguments;
            priority = mv.priority;
            function = mv.function;

            return *this;
        }


        /**
         * @brief Copy operator.
         */
        Function& operator=(const Function& mv)
        {
            name = mv.name;
            numberOfArguments = mv.numberOfArguments;
            priority = mv.priority;
            function = mv.function;

            return (*this);
        }

        // Function name
        std::string name;

        // Number of arguments for function.
        // Using in validation stage.
        uint32_t numberOfArguments;

        // Function priority.
        std::size_t priority;

        // Pointer to function implementation.
        NumberType (*function)(ArgumentsStack&);
    };

    /**
     * @brief Constructor.
     * Default functions:
     * `+` - add
     * `-` - substract
     * `*` - multiply
     * `/` - divide
     * `^` - power
     * `!` - factorial
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
    NumberType execute();

    /**
     * @brief Method for adding basic functions.
     *
     * Functions list:
     * `&` - binary AND.
     * `|` - binary OR.
     * `%` - module.
     * `abs` - absolute value.
     * `sin` - sine.
     * `cos` - cosine.
     * `tan` - tangent.
     * `acos` - arc cosine.
     * `asin` - arc sine.
     * `atan` - arc tangent.
     * `atan2` - arc tangent with 2 params.
     * `cosh` - hyperbolic cosine.
     * `sinh` - hyperbolic sine.
     * `tanh` - hyperbolic tangent.
     * `log` - natural logarithm.
     * `log10` - common logarithm,
     * `sqrt` - square root.
     * `ceil` - round up value.
     * `floor` - round down value.
     */
    void addBasicFunctions();

    /**
     * @brief Method for adding logic functions.
     *
     * Functions list:
     * `if`, `>`, `<`, `>=`, `<=`, `==`, `!=`
     */
    void addLogicFunctions();

    /**
     * @brief Method for adding default constants.
     *
     * Constants list:
     * `PI` - Pi value.
     * `e` - exponent value.
     */
    void addConstants();

    /**
     * @brief Method for setting variable value.
     * Variables can be changed after setting expression.
     * @param name Variable name.
     * @param value Variable value.
     */
    void setVariable(std::string name, NumberType value);

    /**
     * @brief Method for adding constant value.
     * Constant values can not be changed after setting
     * expression.
     * @param name Constant name.
     * @param value Constant variable.
     */
    void addConstant(std::string name, NumberType value);

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

    // Functions by hashes.
    std::map<std::size_t, Function> m_functions;

    // Variable values by hashes.
    std::map<std::size_t, NumberType> m_variables;

    // Constant values by hashes.
    std::map<std::size_t, NumberType> m_constants;

    // Container with parsed expression in revese
    // polish notation.
    LexemStack m_expression;

    // Internal brace counter, that's used in
    // lexem states.
    int m_braceTest;

    // Arguments stack buffer. Used in execution.
    ArgumentsStack m_executionStack;

    // Hash calculator for strings.
    std::hash<std::string_view> m_stringHash;
};

std::ostream& operator<<(std::ostream& stream, const Calculator::LexemStack& stack);