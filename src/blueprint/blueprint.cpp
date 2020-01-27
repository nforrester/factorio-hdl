#include "blueprint.h"
#include "Base64.h"
#include "zlib_easy.h"

#include <cassert>

std::string blueprint_string_to_raw_json(std::string const & blueprint_string)
{
    assert(blueprint_string.size() > 0);
    assert(blueprint_string.at(0) == '0'); // Version byte
    std::string const base64_encoded = blueprint_string.substr(1);

    std::string zlib_compressed;
    assert("" == macaron::Base64::Decode(base64_encoded, zlib_compressed));

    std::string const raw_json = zlib_decompress(zlib_compressed);

    return raw_json;
}

std::string raw_json_to_blueprint_string(std::string const & raw_json)
{
    std::string const zlib_compressed = zlib_compress(raw_json);
    std::string const base64_encoded = macaron::Base64::Encode(zlib_compressed);
    std::string const blueprint_string = "0" + base64_encoded; // Version byte

    return blueprint_string;
}
