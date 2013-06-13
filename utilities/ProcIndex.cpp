#include "ProcIndex.h"

#include "ConfigReader.h"
#include "mpi.h"

ProcIndex::ProcIndex()
{
    MPI_Comm_rank(MPI_COMM_WORLD, &globalIndex);
}

ProcIndex::ProcIndex(int globalIndex)
{
    this->globalIndex = globalIndex;
}

ProcIndex::ProcIndex(const std::vector<int>& regionIndex)
{
    globalIndex = region2global(regionIndex);
}

ProcIndex::~ProcIndex()
{
}

std::vector<int> ProcIndex::getRegionIndex() const
{
    return global2region(this->globalIndex);
}

int ProcIndex::region2global(const std::vector<int>& regionIndex) const
{
    std::vector<int> regionCount = config().GetRegionCount();
    return regionIndex[0]
         + regionCount[0] * regionIndex[1]
         + regionCount[0] * regionCount[1] * regionIndex[2];
}

std::vector<int> ProcIndex::global2region(int globalIndex) const
{
    std::vector<int> regionCount = config().GetRegionCount();
    std::vector<int> regionIndex(3);
    int index = globalIndex;
    regionIndex[0] = index % regionCount[0];
    index /= regionCount[0];
    regionIndex[1] = index % regionCount[1];
    index /= regionCount[1];
    regionIndex[2] = index;
    assert(regionIndex[2] < regionCount[2]);
    return regionIndex;
}
