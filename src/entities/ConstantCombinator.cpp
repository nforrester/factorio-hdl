#include "ConstantCombinator.h"
#include "src/blueprint/Blueprint.h"

void ConstantCombinator::to_blueprint_entity(Blueprint::Entity & bpe) const
{
    bpe.name = Signal::constant_combinator;

    bpe.control_behavior = Blueprint::Entity::Filters();
    auto & f = std::get<Blueprint::Entity::Filters>(*bpe.control_behavior);

    int i = 1;
    for (auto const & iv : constants)
    {
        f.filters.emplace_back(iv.second, i, iv.first);
        ++i;
    }
}
