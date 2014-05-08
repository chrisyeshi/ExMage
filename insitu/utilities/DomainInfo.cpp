#include "DomainInfo.h"

#include "mpi.h"

//
//
// Constructors / Destructors
//
//

//
//
// Public Methods
//
//

int DomainInfo::myRank()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}

Vector<DomainInfo::vDim, int> DomainInfo::myRank3()
{
    return toRank3(myRank());
}

int DomainInfo::toRank(const Vector<vDim, int>& rank3)
{
    return rank3[0] + nRegions3()[0] * rank3[1] + nRegions3()[0] * nRegions3()[1] * rank3[2];
}

Vector<DomainInfo::vDim, int> DomainInfo::toRank3(const int& rank)
{
    Vector<vDim, int> rank3;
    int index = rank;
    rank3[0] = index % nRegions3()[0];
    index /= nRegions3()[0];
    rank3[1] = index % nRegions3()[1];
    index /= nRegions3()[1];
    rank3[2] = index % nRegions3()[2];
    assert(rank3[2] < nRegions3()[2]);
    return rank3;
}

std::vector<Vector<> > DomainInfo::bounds()
{
    return bounds(myRank3());
}

std::vector<Vector<> > DomainInfo::bounds(const int& rank)
{
    return bounds(toRank3(rank));
}

std::vector<Vector<> > DomainInfo::bounds(const Vector<vDim, int>& rank3)
{
    std::vector<Vector<> > ret(2);
    if (!inVolume(rank3))
        return ret;
    for (int i = 0; i < vDim; ++i)
    {
        ret[MIN][i] = volDim()[i] / nRegions3()[i] * (rank3[i] + 0);
        ret[MAX][i] = volDim()[i] / nRegions3()[i] * (rank3[i] + 1);
    }
    return ret;
}

Vector<DomainInfo::vDim> DomainInfo::ranges()
{
    return bounds()[MAX] - bounds()[MIN];
}

Vector<DomainInfo::vDim> DomainInfo::ranges(const int& rank)
{
    return bounds(rank)[MAX] - bounds(rank)[MIN];
}

Vector<DomainInfo::vDim> DomainInfo::ranges(const Vector<vDim, int>& rank3)
{
    return bounds(rank3)[MAX] - bounds(rank3)[MIN];
}

bool DomainInfo::inBounds(const Vector<vDim>& coord)
{
    return inBounds(coord, myRank3());
}

bool DomainInfo::inBounds(const Vector<vDim>& coord, const int& rank)
{
    return inBounds(coord, toRank3(rank));
}

bool DomainInfo::inBounds(const Vector<vDim>& coord, const Vector<vDim, int>& rank3)
{
    if (!inVolume(rank3))
        return false;
    for (int i = 0; i < vDim; ++i)
        if (coord[i] < bounds(rank3)[MIN][i] || coord[i] >= bounds(rank3)[MAX][i])
            return false;
    return true;
}

bool DomainInfo::inVolume(const Vector<vDim, int>& rank3)
{
    for (int i = 0; i < vDim; ++i)
        if (rank3[i] < 0 || rank3[i] >= nRegions3()[i])
            return false;
    return true;
}

bool DomainInfo::inBorder(const Vector<vDim>& coord)
{
    return inBorder(coord, myRank3());
}

bool DomainInfo::inBorder(const Vector<vDim>& coord, const int& rank)
{
    return inBorder(coord, toRank3(rank));
}

bool DomainInfo::inBorder(const Vector<vDim>& coord, const Vector<vDim, int>& rank3)
{
    if (!inVolume(rank3))
        return false;
    if (!inBounds(coord, rank3))
        return false;
    for (int i = 0; i < vDim; ++i)
    {
        if (coord[i] <= (bounds(rank3)[MIN][i] + borderWidth())
         || coord[i] >= (bounds(rank3)[MAX][i] - borderWidth()))
        {
            return true;
        }
    }
    return false;
}

//
//
// Protected Methods
//
//

//
//
// Private Methods
//
//