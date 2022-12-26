#pragma once

#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

template <typename T>
struct ParseSuccess
{
    T value;
    std::string rest;
};

struct ParseFailure
{
    std::string message;
};

template <typename T>
using ParseResult = std::variant<ParseSuccess<T>, ParseFailure>;

template <typename T>
using Parser = std::function<ParseResult<T>(const std::string &)>;

Parser<char> Literal(char c)
{
    return [c](const std::string &input) {
        if (input[0] == c)
        {
            return ParseResult<char>(
                ParseSuccess<char>{char(input[0]), {input.cbegin() + 1, input.cend()}});
        }
        else
        {
            std::stringstream ss;
            ss << "Expected '" << int(c) << "' but got '" << input[0] << "'";
            return ParseResult<char>(ParseFailure{ss.str()});
        }
    };
}

template <typename T>
Parser<T> AndThen(const Parser<T> &a, const Parser<T> &b)
{
    return [&a, &b](const std::string &input) {
        const auto result = a(input);
        if (std::holds_alternative<ParseSuccess<T>>(result))
        {
            const auto &succ = std::get<ParseSuccess<T>>(result);
            return b(succ.rest);
        }
        else
        {
            return ParseResult<T>(result);
        }
    };
}

template <typename T>
Parser<T> OrElse(const Parser<T> &a, const Parser<T> &b)
{
    return [&a, &b](const std::string &input) {
        auto result = a(input);
        if (std::holds_alternative<ParseSuccess<T>>(result))
        {
            return result;
        }
        else
        {
            return ParseResult<T>(b(input));
        }
    };
}

template <typename T>
Parser<T> AnyOf(const std::vector<Parser<T>> &choices)
{
    return [choices](const std::string &input) {
        for (const auto &parser : choices)
        {
            auto result = parser(input);
            if (std::holds_alternative<ParseSuccess<T>>(result))
            {
                return result;
            }
        }
        return ParseResult<T>(ParseFailure{"Could not parse any choices"});
    };
}

template <typename T>
Parser<T> Map(const Parser<T> &parser)
{
    return [&parser](const std::string &input) {};
}
