#pragma once

void check_valid_top_level_form(S::Exp const & form);

/* Do a basic syntax check on the defpart forms and make a map of them. */
std::unordered_map<std::string, S::PtrV const *>
ast_to_defparts(S::PtrV const & ast);

/* Gather args for a new part. */
std::vector<Arg> gather_new_part_args(S::Exp const & body_form);
