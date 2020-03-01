#pragma once

#include "src/host/signals.h"

#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <stdexcept>

namespace S
{
    struct Exp;
    struct List;
    struct Symbol;
    struct String;
    struct Int;

    using Ptr = std::unique_ptr<Exp>;
    using PtrV = std::vector<Ptr>;

    Ptr read(std::string const & input, std::string const & file, size_t line);
    Ptr read(std::string_view & input, std::string const & file, size_t & line);
    struct ParseError;

    PtrV consume(std::string const & input, std::string const & filename, size_t line);
}

struct S::Exp
{
    List * as_list();
    Symbol * as_symbol();
    String * as_string();
    Int * as_int();

    virtual std::string write() const = 0;

    std::string file = "";
    size_t line = 0;
};

struct S::List: public S::Exp
{
    PtrV l;

    std::string write() const override;
};

struct S::Symbol: public S::Exp
{
    Symbol(std::string const & s_): s(s_) {}
    Symbol(std::string_view const & s_): s(s_) {}

    std::string s;

    std::string write() const override { return s; }
};

struct S::String: public S::Exp
{
    String(std::string const & s_): s(s_) {}
    String(std::string_view const & s_): s(s_) {}

    std::string s;

    std::string write() const override;
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
