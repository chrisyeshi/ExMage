#ifndef __ProcIndex_h_
#define __ProcIndex_h_

#include <vector>
#include "ConfigReader.h"

class ProcIndex
{
public:
    ProcIndex();
    ProcIndex(int globalIndex);
    ProcIndex(const std::vector<int>& regionIndex);
    ~ProcIndex();

    int getGlobalIndex() const {return this->globalIndex;}
    std::vector<int> getRegionIndex() const;

protected:
    int region2global(const std::vector<int>& regionIndex) const;
    std::vector<int> global2region(int globalIndex) const;
    ConfigReader& config() const {return ConfigReader::getInstance();}

private:
    int globalIndex;

};

#endif
