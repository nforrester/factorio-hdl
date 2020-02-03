#include "FdlMacro.h"
#include "src/util.h"
#include "src/blueprint/util.h"
#include "util.h"

#include <sstream>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <random>
#include <fstream>

namespace
{
    std::random_device dir_gen_dev;
    std::mt19937 dir_gen_prng(dir_gen_dev());
    std::uniform_int_distribution<uint64_t> dir_gen_rand(0);

	std::filesystem::path make_temp_dir()
	{
        for (int i = 0; i < 1000; ++i)
        {
			std::ostringstream dirname;
			dirname << std::hex << dir_gen_rand(dir_gen_prng);
            std::filesystem::path const dirpath =
                std::filesystem::temp_directory_path() / dirname.str();
			if (std::filesystem::create_directory(dirpath))
            {
				return dirpath;
			}
		}
        throw std::runtime_error("could not find non-existing directory");
	}
}

S::Ptr Fdl::Macro::execute(S::List const & orig_form) const
{
    /* Make a copy, and then quote the arguments. */
    S::Ptr quoted_args = S::read(orig_form.write(), orig_form.file, orig_form.line);
    S::List * ql = quoted_args->as_list();
    assert(ql && ql->l.begin() != ql->l.end());
    for (auto it = ql->l.begin() + 1; it != ql->l.end(); ++it)
    {
        *it = S::read("(quote " + (*it)->write() + ")", orig_form.file, orig_form.line);
    }

    /* Autogenerate some useful definitions. */
    std::filesystem::path temp_dir = make_temp_dir();
    std::filesystem::path autogen_path = temp_dir / "autogen.scm";
    {
        std::ofstream autogen(autogen_path.string());
        assert(autogen.good());
        autogen << "(define all-signals (quote (";
        for (SignalId s = 0; s < num_signals; ++s)
        {
            autogen << " sig:" << get_signal_name_lower_case(s);
        }
        autogen << ")))\n";
    }

    /* Feed it to scheme. */
    std::ostringstream command;
    command << "scm -l '" << autogen_path.string() << "' -l '" << scheme_file << "' ";
    command << "-e '(write " << quoted_args->write() << ")'";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(command.str().c_str(), "r"),
        pclose);

    if (!pipe)
    {
        throw S::ParseError(
            orig_form.file,
            orig_form.line,
            "Error while executing macro " + scheme_function);
    }

    std::string generated_fdl;
    std::array<char, 2048> buf;
    while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr)
    {
        generated_fdl += buf.data();
    }

    std::filesystem::remove_all(temp_dir);

    /* Read, check, and return the replacement form. */
    S::Ptr replacement_form = S::read(generated_fdl, orig_form.file, orig_form.line);
    check_valid_top_level_form(*replacement_form);
    return replacement_form;
}

void Fdl::expand_these_macros(MacroMap const & macros, S::Ptr & ast_node)
{
    S::List * l = ast_node->as_list();
    if (!l || l->l.empty())
    {
        return;
    }

    auto & ll = l->l;
    S::Symbol * front_sym = ll.front()->as_symbol();
    if (front_sym && macros.count(front_sym->s))
    {
        ast_node = macros.at(ll.front()->as_symbol()->s).execute(*l);
        expand_these_macros(macros, ast_node);
        return;
    }

    for (S::Ptr & sub_node : ll)
    {
        expand_these_macros(macros, sub_node);
    }
}

void Fdl::expand_all_macros(S::PtrV & ast, std::string const & filepath)
{
    MacroMap macros;

    auto it = ast.begin();
    while (it != ast.end())
    {
        expand_these_macros(macros, *it);

        S::List * l = (*it)->as_list();
        auto & ll = l->l;
        if (ll.front()->as_symbol() && ll.front()->as_symbol()->s == "defmacro")
        {
            if (ll.size() != 3 || !ll.at(1)->as_symbol() || !ll.at(2)->as_string())
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Expected (defmacro <symbol:scheme_function> <string:scheme_file>).");
            }

            Macro macro;
            macro.scheme_function = ll.at(1)->as_symbol()->s;
            macro.scheme_file = std::filesystem::path(filepath).replace_filename(ll.at(2)->as_string()->s);
            if (macros.count(macro.scheme_function))
            {
                throw S::ParseError(
                    l->file,
                    l->line,
                    "Redefinition of macro " + macro.scheme_function + ".");
            }
            macros[macro.scheme_function] = macro;
            it = ast.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
