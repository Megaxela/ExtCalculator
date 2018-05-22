#include <benchmark/benchmark.h>
#include <Calculator.hpp>
#include <cmath>
#include <sstream>
#include <tinyexpr.h>
#include <gtest/gtest.h>

static void libExecSpeed(benchmark::State& s)
{
    Calculator calc;
    calc.addBasicFunctions();
    
    calc.setExpression("(1/(x+1)+2/(x+2)+3/(x+3))");

    calc.setVariable("x", 5);

    for (auto&& _ : s)
    {
        for (int i = 0; i < s.range(0); ++i)
        {
            auto result = calc.execute();
            benchmark::DoNotOptimize(result);
        }
    }
    
    s.SetComplexityN(s.range(0));
}

static void tinyexprExecSpeed(benchmark::State& s)
{
    double x = 5;

    te_variable vars[] = {{"x", &x}};

    auto expr = te_compile("(1/(x+1)+2/(x+2)+3/(x+3))", vars, 1, nullptr);

    for (auto&& _ : s)
    {
        for (int i = 0; i < s.range(0); ++i)
        {
            auto result = te_eval(expr);
            benchmark::DoNotOptimize(result);
        }
    }

    s.SetComplexityN(s.range(0));

    te_free(expr);
}

static void nativeExecSpeed(benchmark::State& s)
{
    for (auto&& _ : s)
    {
        for (int i = 0; i < s.range(0); ++i)
        {
            auto result = 12 * std::sin(i) + i;
            benchmark::DoNotOptimize(result);
        }
    }
    
    s.SetComplexityN(s.range(0));
}

static void compilationNoOptimization(benchmark::State& s)
{
    std::stringstream ss;

    for (int i = 0; i < s.range(0) - 1; ++i)
    {
        ss << "(1 + 2) *";
    }
    ss << "(1 + 2)";

    auto str = ss.str();

    Calculator calculator;
    calculator.addBasicFunctions();

    for (auto&& _ : s)
    {
        calculator.setExpression(str, false);
    }

    s.SetComplexityN(s.range(0));
}

static void compilationOptimization(benchmark::State& s)
{
    std::stringstream ss;

    for (int i = 0; i < s.range(0) - 1; ++i)
    {
        ss << "(1 + 2) *";
    }
    ss << "(1 + 2)";

    auto str = ss.str();

    Calculator calculator;
    calculator.addBasicFunctions();

    for (auto&& _ : s)
    {
        calculator.setExpression(str, true);
    }

    s.SetComplexityN(s.range(0));
}


BENCHMARK(libExecSpeed)
    ->Range(1, 1U << 20U)
    ->Complexity();

BENCHMARK(tinyexprExecSpeed)
    ->Range(1, 1U << 20U)
    ->Complexity();

BENCHMARK(nativeExecSpeed)
    ->Range(1, 1U << 20U)
    ->Complexity();

BENCHMARK(compilationNoOptimization)
    ->Range(1, 1U << 20U)
    ->Complexity();

BENCHMARK(compilationOptimization)
    ->Range(1, 1U << 20U)
    ->Complexity();

BENCHMARK_MAIN();