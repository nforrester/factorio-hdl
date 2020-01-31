#include "src/Factorio.h"
#include "FdlEntity.h"

#include <iostream>
#include <sstream>

int main(int argc, char ** argv)
{
    assert(argc == 2);

    Factorio fac;

    Entity & fdl = fac.new_entity<Fdl::Entity>("main", std::vector<Fdl::Entity::Arg>(), argv[1]);

    fac.build();

    std::cout << "Begin\n";

    bool stable = false;
    for (int i = 0; true; ++i)
    {
        if (i == 1000 || stable)
        {
            break;
        }

        stable = fac.tick();
        std::cout << "\nTick.\n";
    }

    std::cout << "\n";
    if (stable)
    {
        std::cout << "Stable.";
    }
    else
    {
        std::cout << "Did not stabilize before timeout.";
    }
    std::cout << "\n";
    std::cout << "\n";
    std::cout << "\n";

    std::cout << fac.get_blueprint_string(fdl, argv[1]) << "\n";

    return 0;
}
