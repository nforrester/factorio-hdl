#include "src/Factorio.h"
#include "FdlEntity.h"

#include <iostream>
#include <sstream>

int main(int argc, char ** argv)
{
    assert(argc == 2);

    Factorio fac;

    Entity & fdl = fac.new_entity<Fdl::Entity>(
        "main",
        std::vector<Fdl::Entity::Arg>(),
        std::unordered_map<std::string, std::set<WireColor>>(),
        argv[1]);

    fac.build();

    std::cerr << "Begin\n";

    bool stable = false;
    for (int i = 0; true; ++i)
    {
        if (i == 1000 || stable)
        {
            break;
        }

        stable = fac.tick();
        std::cerr << "Tick.\n";
    }

    if (stable)
    {
        std::cerr << "Stable.";
    }
    else
    {
        std::cerr << "Did not stabilize before timeout.";
    }
    std::cerr << "\n";
    std::cerr << "\n";
    std::cerr << "\n";

    std::cout << fac.get_blueprint_string(fdl, argv[1]) << "\n";

    return 0;
}
