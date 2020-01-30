#pragma once

#include "src/Composite.h"

namespace Fdl
{
    class Entity;
}

class Fdl::Entity: public Composite
{
public:
    Entity(Factorio & factorio,
           std::string const & part_name,
           std::string const & fdl_filename);

private:
};
