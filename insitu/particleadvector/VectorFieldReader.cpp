#include "VectorFieldReader.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include "ConfigReader.h"
#include "DomainInfo.h"

extern "C" {
    int readData(const char* filename,
                 int region_index[3], int region_count[3],
                 char attributes[][50], int attribute_count,
                 float* flow_field[6]);
}

VectorFieldReader::VectorFieldReader()
{
}

VectorFieldReader::~VectorFieldReader()
{
    for (unsigned int i = 0; i < fields.size(); ++i)
    {
        delete [] fields[i];
        fields[i] = NULL;
    }
    fields.clear();
}

bool VectorFieldReader::read()
{
    ConfigReader& config = ConfigReader::getInstance();
    // hdf5
    if (config.get("input.format").asString() == "hdf5")
        return readHdf5();

    // shouldn't reach here
    return false;
}

bool VectorFieldReader::readHdf5()
{
    ConfigReader& config = ConfigReader::getInstance();
    std::string filename = config.get("input.file").asString();

    // attributes
    std::vector<std::string> input_attributes = config.get("input.attributes").asArray<std::string, std::string>();
    int attribute_count = input_attributes.size();
    fields.resize(attribute_count);
    char attributes[attribute_count][50];
    for (int i = 0; i < attribute_count; ++i)
        sprintf(attributes[i], "%s", input_attributes[i].c_str());

    // region information
    std::vector<int> region_count = config.get("domain.count").asArray<double, int>();
    Vector<3, int> region_index = DomainInfo::myRank3();

    // read
    int ret = readData(filename.c_str(),
                       &region_index[0], &region_count[0],
                       attributes, attribute_count,
                       &fields[0]);
    return (ret == 1) ? true : false;
}

std::string VectorFieldReader::ext() const
{
    return ConfigReader::getInstance().get("input.format").asString();
}

std::vector<int> VectorFieldReader::timestepRange() const
{
    return ConfigReader::getInstance().get("input.time").asArray<double, int>();
}
