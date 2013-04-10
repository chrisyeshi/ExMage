/**
 * Test the json reader
 */

#include <fstream>
#include <cassert>

#include "../picojson.h"

int main(void)
{
    picojson::value v;

    std::ifstream fin("configure.json");
    fin >> v;

    assert(v.is<picojson::object>());
    const picojson::object& o = v.get<picojson::object>();
    for (picojson::object::const_iterator i = o.begin(); i != o.end(); ++i)
    {
        std::cout << i->first << "  " << i->second << std::endl;
    }

    picojson::value junk = v.get("kshdfsl"); // shoudln't error out
    assert(junk.is<picojson::null>());
    if (junk.is<picojson::null>())
        std::cout << "Junk test: Pass!" << std::endl;

    assert(!v.contains("junk"));
    if (!v.contains("junk"))
        std::cout << "Contain test: Pass!" << std::endl;

    return 0;
}
