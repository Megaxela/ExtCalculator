#pragma once

#include <stdexcept>

struct CalculationException : std::runtime_error
{
    explicit CalculationException(const std::string& string) :
        std::runtime_error(string)
    {

    }
};