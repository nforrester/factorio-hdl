#pragma once

#include "src/signals.h"

#include <string>
#include <string_view>
#include <memory>
#include <forward_list>
#include <stdexcept>

namespace S
{
    struct Exp;
    struct List;
    struct Symbol;
    struct Int;

    std::unique_ptr<Exp> read(std::string const & input, std::string const & file, size_t line);
    std::unique_ptr<Exp> read(std::string_view & input, std::string const & file, size_t & line);
    struct ParseError;
}

struct S::Exp
{
    List * as_list();
    Symbol * as_symbol();
    Int * as_int();

    virtual std::string write() const = 0;
};

struct S::List: public S::Exp
{
    using Type = std::forward_list<std::unique_ptr<Exp>>;
    Type l;

    std::string write() const override;
};

struct S::Symbol: public S::Exp
{
    Symbol(std::string const & s_): s(s_) {}
    Symbol(std::string_view const & s_): s(s_) {}

    std::string s;

    std::string write() const override { return s; }
};

struct S::Int: public S::Exp
{
    Int(SignalValue v_): v(v_) {}

    SignalValue v;

    std::string write() const override { return std::to_string(v); }
};

struct S::ParseError: public std::runtime_error
{
    ParseError(std::string const & file_, size_t line_, std::string const & what):
        std::runtime_error(file_ + ":" + std::to_string(line_) + ": " + what),
        file(file_),
        line(line_)
    {
    }

    std::string file;
    size_t line;
};
