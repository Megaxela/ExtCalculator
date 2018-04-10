# Expression Calculator
It's pure C++17 based extendable expression calculator.

## Build
It's CMake based project. Here is the steps how to build it.

1. Clone repo: `git clone https://github.com/Megaxela/ExtCalculator`.
1. Go into cloned repo: `cd ExtCalculator`
    1. If you want to build tests and benchmarks: `git submodule update --init`
1. Create build folder: `mkdir build`
1. Go into build folder `cd build`
1. Setup project: `cmake <DEFINES_HERE> ..`.
    1. If you want to build tests or tests with benchmarks `--DCALC_BUILD_TESTS=On` or `-DCALC_BUILD_BENCH=On`.
1. Build library: `cmake --build .`.

## Example:
```cpp
#include "Calculator.hpp"

int main()
{
    // Creating calculator instance
    Calculator calculator;

    // Adding base function, like "+, -, *, /, sin, cos, etc..."
    calculator.addBasicFunctions();

    // Setting expression
    try
    {
        calculator.setExpression("12 / 22 / x * sin(y)");
    }
    catch (ParsingException& e)
    {
        std::cout << "Parsing exception: " << e.what() << std::endl << std::endl;
        return 1;
    }
    catch (StatementException& e)
    {
        std::cout << "Statement exception: " << e.what() << std::endl << std::endl;
        return 1;
    }

    // Setting variables values
    calculator.setVariable("x", 0.2);
    calculator.setVariable("y", 3.14 / 2);

    // Executing expression
    try
    {
        std::cout << "Result: " << calculator.execute() << std::endl;
    }
    catch (CalculationException& e)
    {
        std::cout << "Calculation exception: " << e.what() << std::endl << std::endl;
        return 1;
    }

    // You are able to get internal RPN after parsing and optimizations.
    Calculator::LexemStack stack;

    calculator.getRPN(stack);

    // Output: "Internal RPN: 0.545455 x / y sin *"
    std::cout << "Internal RPN: " << stack << std::endl;

    // Also you are able to add your own functions.
    // Just like this.
    calculator.addFunction(
        Calculator::Function(
            "sample_function",
            2, // Number of arguments
            4, // Priority
            [](Calculator::LexemStack& stack) -> double
            {
                auto second = std::get<double>(stack.back().value);
                stack.pop_back();

                auto first = std::get<double>(stack.back().value);
                stack.pop_back();

                return second + first;
            }
        )
    );

    // Setting new expression
    try
    {
        calculator.setExpression("12 + 11 * sample_function(1, x)");
    }
    catch (ParsingException& e)
    {
        std::cout << "Parsing exception: " << e.what() << std::endl << std::endl;
        return 1;
    }
    catch (StatementException& e)
    {
        std::cout << "Statement exception: " << e.what() << std::endl << std::endl;
        return 1;
    }

    // Executing it
    try
    {
        // Output: "Result: 25.2"
        std::cout << "Result: " << calculator.execute() << std::endl;
    }
    catch (CalculationException& e)
    {
        std::cout << "Calculation exception: " << e.what() << std::endl << std::endl;
        return 1;
    }

    return 0;
}
```

## License
<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

Library is licensed under the [MIT License](https://opensource.org/licenses/MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.