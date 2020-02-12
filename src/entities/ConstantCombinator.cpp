#include "ConstantCombinator.h"
#include "src/blueprint/Blueprint.h"

int ConstantCombinator::to_blueprint_entity(Blueprint::Entity & bpe) const
{
    bpe.name = Signal::constant_combinator;

    bpe.control_behavior.emplace();
    bpe.control_behavior->spec = Blueprint::Entity::Filters();
    auto & f = std::get<Blueprint::Entity::Filters>(*bpe.control_behavior->spec);

    int i = 1;
    for (auto const & iv : constants)
    {
        f.filters.emplace_back(iv.second, i, iv.first);
        ++i;
    }
    assert(f.filters.size() <= 18);

    return 1;
}
