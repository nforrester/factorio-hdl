#include "SExp.h"

#include <unordered_set>
#include <cassert>
#include <sstream>

S::List * S::Exp::as_list()
{
    return dynamic_cast<List*>(this);
}

S::Symbol * S::Exp::as_symbol()
{
    return dynamic_cast<Symbol*>(this);
}

S::Int * S::Exp::as_int()
{
    return dynamic_cast<Int*>(this);
}

namespace
{
    std::unordered_set<char> str2set(std::string const & str)
    {
        return std::unordered_set<char>(str.begin(), str.end());
    }

    std::string const non_line_ending_whitespace_chars_ = " \t";
    std::string const line_ending_whitespace_chars_ = "\n";
    std::string const numeric_start_chars_ = "-0123456789";
    std::string const numeric_second_chars_ = "0123456789";
    std::string const numeric_tail_chars_ = "0123456789xXabcdefABCDEF";
    std::string const symbol_chars_ =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "!=<>+-*/%^|&_";

    std::unordered_set<char> const non_line_ending_whitespace_chars =
        str2set(non_line_ending_whitespace_chars_);
    std::unordered_set<char> const line_ending_whitespace_chars =
        str2set(line_ending_whitespace_chars_);
    std::unordered_set<char> const whitespace_chars =
        str2set(non_line_ending_whitespace_chars_ + line_ending_whitespace_chars_);

    std::unordered_set<char> const numeric_start_chars = str2set(numeric_start_chars_);
    std::unordered_set<char> const numeric_second_chars = str2set(numeric_second_chars_);
    std::unordered_set<char> const numeric_tail_chars = str2set(numeric_tail_chars_);

    std::unordered_set<char> const symbol_chars =
        str2set(symbol_chars_);

    std::unordered_set<char> const post_atomic_chars =
        str2set(line_ending_whitespace_chars_ + non_line_ending_whitespace_chars_ + ")");
}

std::unique_ptr<S::Exp> S::read(std::string const & input, std::string const & file, size_t line)
{
    std::string_view v(input.data(), input.size());
    return read(v, file, line);
}

std::unique_ptr<S::Exp> S::read(std::string_view & input, std::string const & file, size_t & line)
{
    std::unique_ptr<List> list;
    List::Type::iterator tail;
    while (true)
    {
        if (input.empty())
        {
            if (list)
            {
                throw ParseError(file, line, "Unterminated list.");
            }
            return std::unique_ptr<Exp>();
        }

        if (non_line_ending_whitespace_chars.count(input.front()))
        {
            input.remove_prefix(1);
            continue;
        }

        if (line_ending_whitespace_chars.count(input.front()))
        {
            input.remove_prefix(1);
            ++line;
            continue;
        }

        assert(!whitespace_chars.count(input.front()));

        /* Comment, advance to end of line. */
        if (input.front() == ';')
        {
            while (!line_ending_whitespace_chars.count(input.front()))
            {
                if (input.empty())
                {
                    break;
                }
                input.remove_prefix(1);
            }
            continue;
        }

        if (list)
        {
            if (input.front() == ')')
            {
                input.remove_prefix(1);
                return list;
            }
            std::unique_ptr<Exp> elem = read(input, file, line);
            assert(elem);
            if (tail == list->l.end())
            {
                list->l.emplace_front(std::move(elem));
                tail = list->l.begin();
            }
            else
            {
                list->l.emplace_after(tail, std::move(elem));
                ++tail;
            }
            continue;
        }

        bool its_a_trap =
            (input.front() == '-') && 
            (input.size() == 1 ||
             numeric_second_chars.count(input.at(1)) == 0);
        if (!its_a_trap && numeric_start_chars.count(input.front()))
        {
            std::string_view input_at_start_of_number = input;
            std::string_view numeric_string;
            while (!input.empty() && !post_atomic_chars.count(input.front()))
            {
                bool first = numeric_string.size() == 0;
                if (!((first && numeric_start_chars.count(input.front())) ||
                      (!first && numeric_tail_chars.count(input.front()))))
                {
                    throw ParseError(file, line, std::string("Character '") + input.front() + "' is not valid in an int.");
                }

                numeric_string = std::string_view(input_at_start_of_number.data(),
                                                  numeric_string.size() + 1);
                input.remove_prefix(1);
            }
            try
            {
                size_t n_consumed;
                intmax_t parsed = std::stoll(std::string(numeric_string), &n_consumed, 0);
                if (n_consumed != numeric_string.size())
                {
                    throw std::invalid_argument("");
                }
                if (parsed > std::numeric_limits<SignalValue>::max() ||
                    parsed < std::numeric_limits<SignalValue>::lowest())
                {
                    throw std::out_of_range("");
                }
                return std::make_unique<Int>(static_cast<SignalValue>(parsed));
            }
            catch (std::invalid_argument & e)
            {
                throw ParseError(file, line, "Not a valid integer: \"" + std::string(numeric_string) + "\".");
            }
            catch (std::out_of_range & e)
            {
                throw ParseError(file, line, "This integer is too long: \"" + std::string(numeric_string) + "\".");
            }
        }

        if (symbol_chars.count(input.front()))
        {
            std::string_view input_at_start_of_symbol = input;
            std::string_view symbol;
            while (!input.empty() && !post_atomic_chars.count(input.front()))
            {
                if (!symbol_chars.count(input.front()))
                {
                    throw ParseError(file, line, std::string("Character '") + input.front() + "' is not valid in a symbol.");
                }
                symbol = std::string_view(input_at_start_of_symbol.data(), symbol.size() + 1);
                input.remove_prefix(1);
            }
            return std::make_unique<Symbol>(symbol);
        }

        if (input.front() == '(')
        {
            list = std::make_unique<List>();
            tail = list->l.begin();
            input.remove_prefix(1);
            continue;
        }

        throw ParseError(file, line, std::string("Expected int, symbol, or list. Found '") + input.front() + "'.");
    }
}

std::string S::List::write() const
{
    std::ostringstream out;
    out << '(';
    bool first = true;
    for (std::unique_ptr<S::Exp> const & s : l)
    {
        assert(s);
        if (!first)
        {
            out << ' ';
        }
        first = false;
        out << s->write();
    }
    out << ')';
    return out.str();
}
