#include "SExp.h"
#include "src/util.h"

#include <unordered_set>
#include <unordered_map>
#include <sstream>

S::List * S::Exp::as_list()
{
    return dynamic_cast<List*>(this);
}

S::Symbol * S::Exp::as_symbol()
{
    return dynamic_cast<Symbol*>(this);
}

S::String * S::Exp::as_string()
{
    return dynamic_cast<String*>(this);
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
        "!=<>+-*/%^|&_:";

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

    std::unordered_map<char, char> const string_escape_map =
    {
        {'\\', '\\'},
        {'"', '"'},
        {'n', '\n'},
        {'t', '\t'},
    };
}

S::Ptr S::read(std::string const & input, std::string const & file, size_t line)
{
    std::string_view v(input.data(), input.size());
    S::Ptr result = read(v, file, line);
    if (read(v, file, line))
    {
        throw ParseError(file, line, "read(string,...) called on a string with multiple forms.");
    }
    return result;
}

S::Ptr S::read(std::string_view & input, std::string const & file, size_t & line)
{
    std::unique_ptr<List> list;
    while (true)
    {
        if (input.empty())
        {
            if (list)
            {
                throw ParseError(file, line, "Unterminated list.");
            }
            return Ptr();
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
            Ptr elem = read(input, file, line);
            assert(elem);
            list->l.emplace_back(std::move(elem));
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
                Ptr e = std::make_unique<Int>(static_cast<SignalValue>(parsed));
                e->file = file;
                e->line = line;
                return e;
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
            Ptr s = std::make_unique<Symbol>(symbol);
            s->file = file;
            s->line = line;
            return s;
        }

        if (input.front() == '"')
        {
            input.remove_prefix(1);
            if (input.empty())
            {
                throw ParseError(file, line, "Unterminated string.");
            }

            std::string s;
            while (input.front() != '"')
            {
                if (input.front() == '\\')
                {
                    input.remove_prefix(1);
                    if (input.empty())
                    {
                        throw ParseError(file, line, "Unterminated string.");
                    }
                    if (string_escape_map.count(input.front()) == 0)
                    {
                        throw ParseError(file, line, std::string("Unrecognized string escape sequence: \"\\") + input.front() + "\"");
                    }
                    s += string_escape_map.at(input.front());
                }
                else
                {
                    s += input.front();
                }
                input.remove_prefix(1);
                if (input.empty())
                {
                    throw ParseError(file, line, "Unterminated string.");
                }
            }
            input.remove_prefix(1);
            Ptr e = std::make_unique<String>(s);
            e->file = file;
            e->line = line;
            return e;
        }

        if (input.front() == '(')
        {
            list = std::make_unique<List>();
            list->file = file;
            list->line = line;
            input.remove_prefix(1);
            continue;
        }

        throw ParseError(file, line, std::string("Expected int, symbol, string, or list. Found '") + input.front() + "'.");
    }
}

std::string S::List::write() const
{
    std::ostringstream out;
    out << '(';
    bool first = true;
    for (Ptr const & s : l)
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

std::string S::String::write() const
{
    std::string result;
    result += '"';
    for (char c : s)
    {
        for (auto const & kv : string_escape_map) // TODO this is inefficient
        {
            if (kv.second == c)
            {
                result += '\\';
                c = kv.first;
                break;
            }
        }
        result += c;
    }
    result += '"';
    return result;
}

S::PtrV S::consume(std::string const & input, std::string const & filename, size_t line)
{
    std::string_view view(input.data(), input.size());

    S::PtrV ast;
    while (true)
    {
        assert(view.size() > 0);
        S::Ptr e = S::read(view, filename, line);
        if (e)
        {
            ast.push_back(std::move(e));
        }
        else
        {
            assert(view.size() == 0);
            break;
        }
    }
    return ast;
}
