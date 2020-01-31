#pragma once

#include "src/Composite.h"
#include "FdlInstantiatedPart.h"

namespace Fdl
{
    class Entity;
}

class Fdl::Entity: public Composite
{
public:
    using Arg = InstantiatedPart::Arg;

    Entity(Factorio & factorio,
           std::string const & part_name,
           std::vector<Arg> const & provided_args,
           std::unordered_map<std::string, std::set<WireColor>> const & colors_of_outside_wires,
           std::string const & fdl_filename);

private:
    friend class InstantiatedPart;
};
