#pragma once

#include <stdexcept>

struct ParsingException : std::runtime_error
{
    explicit ParsingException(const std::string& string) :
        std::runtime_error(string)
    {

    }
};