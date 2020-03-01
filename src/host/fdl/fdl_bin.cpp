#include "src/host/Factorio.h"
#include "src/host/debug.h"
#include "FdlEntity.h"

#include <sstream>

int main(int argc, char ** argv)
{
    assert(argc == 2);

    Factorio fac;

    Entity & fdl = fac.new_entity<Fdl::Entity>(
        "main > ",
        "main",
        std::vector<Fdl::Arg>(),
        std::unordered_map<std::string, std::set<WireColor>>(),
        argv[1]);

    fac.build();

    info(0) << fac.get_blueprint_string(fdl, argv[1]) << "\n";

    return 0;
}
