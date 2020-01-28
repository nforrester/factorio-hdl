#pragma once

class Blueprint;

void move_blueprint(Blueprint & blueprint, double x, double y);
void merge_blueprints(Blueprint & dst, Blueprint const & src);
void arrange_blueprint_6x7_cell(Blueprint & blueprint);
