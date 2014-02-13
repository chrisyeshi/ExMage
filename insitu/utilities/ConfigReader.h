#ifndef __ConfigReader_h
#define __ConfigReader_h

#include <string>
#include "picojson.h"
#include "CameraCore.h"
#include "Vector.h"

class ConfigValue
{
public:
    ConfigValue() {}
    ConfigValue(const picojson::value& value) : v(value) {}

    CameraCore asCamera() const;
    template<typename in, typename out>
    std::vector<out> asArray() const;
    template<typename type>
    type asNumber() const;
    std::string asString() const;

protected:

private:
    picojson::value v;
};

template<typename in, typename out>
std::vector<out> ConfigValue::asArray() const
{
    if (!v.is<picojson::array>())
        return std::vector<out>();
    const picojson::array& arr = v.get<picojson::array>();
    std::vector<out> ret(arr.size());
    for (unsigned int i = 0; i < arr.size(); ++i)
    {
        ret[i] = arr[i].get<in>();
    }
    return ret;
}

template <typename type>
type ConfigValue::asNumber() const
{
    if (!v.is<double>())
        return -1;
    return type(v.get<double>());
}

class ConfigReader
{
public:
    // Singleton
    static ConfigReader& getInstance()
    {
        static ConfigReader instance(filename);
        return instance;
    }
    static void setFile(const std::string& fn)
    {
        filename = fn;
    }

    ConfigValue get(const std::string& path) const;
    // void set(const std::string& path, double value);
    // void set(const std::string& path, int value);
    // void set(const std::string& path, const CameraCore& camera);
    // void set(const std::string& path, const Vector<dim, type>& value);

protected:
    const picojson::value& get(const std::vector<std::string>& keys, int index, const picojson::value& parent) const;
    std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string> &elems) const;
    std::vector<std::string> split(const std::string& s, char delim) const;

private:
    static std::string filename;
    picojson::value v;

    ConfigReader(); // Don't implement
    ConfigReader(const std::string& filename);
    ConfigReader(const ConfigReader&);      // Don't implement
    void operator=(const ConfigReader&);    // Don't implement
};

#endif
