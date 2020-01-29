#pragma once

namespace Blueprint {
    struct Blueprint;

    void move_blueprint(Blueprint & blueprint, double x, double y);
    void merge_blueprints(Blueprint & dst, Blueprint const & src);

    /* Set position and direction for all entities in blueprint.
     * Blueprint should only have combinators.
     * 6 medium electric poles will be added.
     *
     * Constant combinators whose signals are all 0
     * are assumed to mark exernal interfaces. */
    void arrange_blueprint_6x7_cell(Blueprint & blueprint);
}
