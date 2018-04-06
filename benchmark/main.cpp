#include <benchmark/benchmark.h>
#include <Calculator.hpp>
#include <cmath>

static void libExecSpeed(benchmark::State& s)
{
    Calculator calc;
    calc.addBasicFunctions();
    
    calc.setExpression("12 * sin(x) + x");

    calc.setVariable("x", 55);

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



BENCHMARK(libExecSpeed)
    ->Range(1, 10000)
    ->Complexity();

BENCHMARK(nativeExecSpeed)
    ->Range(1, 10000)
    ->Complexity();

BENCHMARK_MAIN();