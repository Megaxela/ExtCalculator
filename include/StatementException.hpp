#pragma once

#include <stdexcept>

struct StatementException : std::runtime_error
{
    explicit StatementException(const std::string& string) :
        std::runtime_error(string)
    {

    }
};