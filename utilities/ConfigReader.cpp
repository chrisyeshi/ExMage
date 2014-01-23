#include "ConfigReader.h"

#include <fstream>
#include <sstream>

std::string ConfigReader::filename = "configure.json";

CameraCore ConfigValue::asCamera() const
{
    CameraCore camera;
    if (!v.contains("position") || !v.contains("focal") || !v.contains("up"))
        return camera;
    picojson::array p = v.get("position").get<picojson::array>();
    picojson::array f = v.get("focal").get<picojson::array>();
    picojson::array u = v.get("up").get<picojson::array>();
    camera.Position   = Vector<3>(p[0].get<double>(), p[1].get<double>(), p[2].get<double>());
    camera.Focal      = Vector<3>(f[0].get<double>(), f[1].get<double>(), f[2].get<double>());
    camera.ViewUp     = Vector<3>(u[0].get<double>(), u[1].get<double>(), u[2].get<double>());
    return camera;
}

std::string ConfigValue::asString() const
{
    if (!v.is<std::string>())
        return std::string();
    return v.get<std::string>();
}

ConfigReader::ConfigReader(const std::string& filename)
{
    std::ifstream fin(filename.c_str());
    fin >> v;
}

ConfigValue ConfigReader::get(const std::string& path) const
{
    std::vector<std::string> keys = split(path, '.');
    const picojson::value& leaf = this->get(keys, 0, v);
    return ConfigValue(leaf);
}

const picojson::value& ConfigReader::get(const std::vector<std::string>& keys, int index, const picojson::value& parent) const
{
    if (index >= keys.size())
        return parent;
    std::string key = keys[index];
    if (!parent.contains(key))
    {
        static picojson::value s_null;
        return s_null;
    }
    const picojson::value& child = parent.get(key);
    return this->get(keys, ++index, child);
}

std::vector<std::string>& ConfigReader::split(const std::string& s, char delim, std::vector<std::string> &elems) const
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> ConfigReader::split(const std::string& s, char delim) const
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
