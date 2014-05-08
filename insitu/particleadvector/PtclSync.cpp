#include <map>

#include "PtclSync.h"
#include "cassert"
#include "mpi.h"
#include "DomainInfo.h"

PtclSync::PtclSync()
{
}

void PtclSync::scatter(const PtclArr& currBorder, const PtclArr& nextBorder, const PtclArr& curr, const PtclArr& next, const VectorField<>& flow)
{
    mapBorder(currBorder, nextBorder);
	mapPtcls(curr, next);
	this->flow = flow;
	this->scatter();
}

PtclSync::PtclArr PtclSync::getCurrOutBorder() const
{
    PtclArr ret;
    PtclMap::const_iterator itr = currOutBorder.begin();
    for (; itr != currOutBorder.end(); ++itr)
        ret.insert(ret.end(), itr->second.begin(), itr->second.end());
    return ret;
}

PtclSync::PtclArr PtclSync::getNextOutBorder() const
{
    PtclArr ret;
    PtclMap::const_iterator itr = nextOutBorder.begin();
    for (; itr != nextOutBorder.end(); ++itr)
        ret.insert(ret.end(), itr->second.begin(), itr->second.end());
    return ret;
}

PtclSync::PtclArr PtclSync::getCurrOut() const
{
    PtclArr ret;
    PtclMap::const_iterator itr = currOut.begin();
    for (; itr != currOut.end(); ++itr)
        ret.insert(ret.end(), itr->second.begin(), itr->second.end());
    return ret;
}

PtclSync::PtclArr PtclSync::getNextOut() const
{
    PtclArr ret;
    PtclMap::const_iterator itr = nextOut.begin();
    for (; itr != nextOut.end(); ++itr)
        ret.insert(ret.end(), itr->second.begin(), itr->second.end());
    return ret;
}

PtclSync::PtclArr PtclSync::getCurrInc() const
{
	PtclArr ret;
	PtclMap::const_iterator itr = currInc.begin();
	for (; itr != currInc.end(); ++itr)
		ret.insert(ret.end(), itr->second.begin(), itr->second.end());
	return ret;
}

PtclSync::PtclArr PtclSync::getNextInc() const
{
	PtclArr ret;
	PtclMap::const_iterator itr = nextInc.begin();
	for (; itr != nextInc.end(); ++itr)
		ret.insert(ret.end(), itr->second.begin(), itr->second.end());
	return ret;
}

void PtclSync::mapBorder(const PtclArr& currBorder, const PtclArr& nextBorder)
{
    currInBorder.clear();
    nextInBorder.clear();
    assert(currBorder.size() == nextBorder.size());
    for (unsigned int i = 0; i < currBorder.size(); ++i)
    {
        // neighbor3 should capture the corner borders
        Vector<vDim, int> neighbor3(0, 0, 0);
        for (int j = 0; j < vDim; ++j)
        {
            // nei3 should capture the side borders
            Vector<vDim, int> nei3(0, 0, 0);
            if (currBorder[i].coord()[j] <= (DomainInfo::bounds()[0][j] + DomainInfo::borderWidth())
             || nextBorder[i].coord()[j] <= (DomainInfo::bounds()[0][j] + DomainInfo::borderWidth()))
            {
                neighbor3[j] = -1;
                nei3[j] = -1;
            } else
            if (currBorder[i].coord()[j] >= (DomainInfo::bounds()[1][j] - DomainInfo::borderWidth())
             || nextBorder[i].coord()[j] >= (DomainInfo::bounds()[1][j] - DomainInfo::borderWidth()))
            {
                neighbor3[j] = 1;
                nei3[j] = 1;
            } else
            {
                continue;
            }
            // insert the particle into neighbor particle map
            currInBorder[nei3].push_back(currBorder[i]);
            nextInBorder[nei3].push_back(nextBorder[i]);
            if (neighbor3 != nei3)
            {
                currInBorder[neighbor3].push_back(currBorder[i]);
                nextInBorder[neighbor3].push_back(nextBorder[i]);
            }
        }
    }
}

void PtclSync::mapPtcls(const PtclArr& curr, const PtclArr& next)
{
	currOut.clear();
	nextOut.clear();
    assert(curr.size() == next.size());
	for (unsigned int i = 0; i < next.size(); ++i)
	{
		Vector<vDim, int> neighbor3(0, 0, 0);
		for (int j = 0; j < vDim; ++j)
		{
			if (next[i].coord()[j] < DomainInfo::bounds()[0][j])
				neighbor3[j] = -1;
			else if (next[i].coord()[j] >= DomainInfo::bounds()[1][j])
				neighbor3[j] = 1;
		}
		assert((neighbor3 != Vector<vDim, int>(0, 0, 0)));
		currOut[neighbor3].push_back(curr[i]);
		nextOut[neighbor3].push_back(next[i]);
	}
}

void PtclSync::scatter()
{
    currOutBorder.clear();
    nextOutBorder.clear();
	currInc.clear();
	nextInc.clear();
    // pre round: send border particles
    // send
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    for (int z = -1; z <= 1; ++z)
    {
        Vector<vDim, int> neighbor3(x, y, z);
        send(neighbor3, currInBorder[neighbor3]);
        send(neighbor3, nextInBorder[neighbor3]);
    }
    // pre round receive
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    for (int z = -1; z <= 1; ++z)
    {
        Vector<vDim, int> neighbor3(x, y, z);
        currOutBorder[neighbor3] = recv(neighbor3);
        nextOutBorder[neighbor3] = recv(neighbor3);
    }
	// first round: send current and next particles
    // send
	for (int x = -1; x <= 1; ++x)
	for (int y = -1; y <= 1; ++y)
	for (int z = -1; z <= 1; ++z)
	{
		Vector<vDim, int> neighbor3(x, y, z);
		send(neighbor3, currOut[neighbor3]);
		send(neighbor3, nextOut[neighbor3]);
	}
	// first round receive
	for (int x = -1; x <= 1; ++x)
	for (int y = -1; y <= 1; ++y)
	for (int z = -1; z <= 1; ++z)
	{
		Vector<vDim, int> neighbor3(x, y, z);
		currInc[neighbor3] = recv(neighbor3);
		nextInc[neighbor3] = recv(neighbor3);
		for (unsigned int i = 0; i < nextInc[neighbor3].size(); ++i)
			nextInc[neighbor3][i].scalars() = flow.getScalars(nextInc[neighbor3][i].coord() - DomainInfo::bounds()[0]);
	}
	// second round ---- GO!!!
	// Here we return the scalar values of the nextInc particles
	// 1. send them back
	for (int x = -1; x <= 1; ++x)
	for (int y = -1; y <= 1; ++y)
	for (int z = -1; z <= 1; ++z)
	{
		Vector<vDim, int> neighbor3(x, y, z);
		send(neighbor3, nextInc[neighbor3]);
	}
	// 2. receive from neighbors
	for (int x = -1; x <= 1; ++x)
	for (int y = -1; y <= 1; ++y)
	for (int z = -1; z <= 1; ++z)
	{
		Vector<vDim, int> neighbor3(x, y, z);
		nextOut[neighbor3] = recv(neighbor3);
	}
}

void PtclSync::send(const Vector<vDim, int> neighbor3, const PtclArr& ptcls) const
{
	// don't send stuff to myself
	if (neighbor3 == Vector<vDim, int>(0, 0, 0))
		return;
	Vector<vDim, int> neiRank3 = DomainInfo::myRank3() + neighbor3;
	if (!DomainInfo::inVolume(neiRank3))
	{
		// TODO: add codes to fill scalars for the nextOut particles
		return;
	}
	// send particle count to neighbor
	int neiRank = DomainInfo::toRank(neiRank3);
	unsigned int count = ptcls.size();
	MPI_Send(&count, 1, MPI_INT, neiRank, Tag_Count, MPI_COMM_WORLD);
	// if no particle, then stop
	if (count == 0)
		return;
#ifdef DEBUG_TEXT
	std::cout << "Send: Proc " << DomainInfo::myRank() << " is sending " << count << " particles to Proc" << neiRank << std::endl;
#endif
	// otherwise send the actual particles data
	// first send the ids
	std::vector<unsigned int> ids(count);
	for (unsigned int i = 0; i < count; ++i)
		ids[i] = ptcls[i].id();
	MPI_Send(ids.data(), count, MPI_UNSIGNED, neiRank, Tag_Ids, MPI_COMM_WORLD);
	// then send the coords and scalars
	unsigned int vDim = flow.nVelocities();
	unsigned int nScalars = flow.nScalars();
	unsigned int nFields = flow.nFields();
	std::vector<float> values(count * nFields);
	for (unsigned int i = 0; i < count; ++i)
	{
#ifdef DEBUG_TEXT
		std::cout << "Send: Proc " << DomainInfo::myRank() << ": " << ptcls[i].coord() << std::endl;
#endif
		for (unsigned int j = 0; j < vDim; ++j)
			values[i * nFields + j] = ptcls[i].coord()[j];
		for (unsigned int j = 0; j < nScalars; ++j)
			values[i * nFields + vDim + j] = ptcls[i].scalar(j);
	}
	MPI_Send(values.data(), count * nFields, MPI_FLOAT, neiRank, Tag_Ptcls, MPI_COMM_WORLD);
}

PtclSync::PtclArr PtclSync::recv(const Vector<vDim, int> neighbor3) const
{
	// not receiving any stuff from myself...
	if (neighbor3 == Vector<vDim, int>(0, 0, 0))
		return PtclArr();
	Vector<vDim, int> neiRank3 = DomainInfo::myRank3() + neighbor3;
	if (!DomainInfo::inVolume(neiRank3))
		return PtclArr();
	// receive particle count from neighbor
	int neiRank = DomainInfo::toRank(neiRank3);
	MPI_Status status;
	unsigned int count;
	MPI_Recv(&count, 1, MPI_UNSIGNED, neiRank, Tag_Count, MPI_COMM_WORLD, &status);
	// if no particle, then stop
	if (count == 0)
		return PtclArr();
#ifdef DEBUG_TEXT
	std::cout << "Recv: Proc " << DomainInfo::myRank() << " is receiving " << count << " particles from Proc" << neiRank << std::endl;
#endif
	// otherwise receive actual particles data
	// first receive the ids
	std::vector<unsigned int> ids(count);
	MPI_Recv(ids.data(), count, MPI_UNSIGNED, neiRank, Tag_Ids, MPI_COMM_WORLD, &status);
	// then receive the coords and scalars
	unsigned int vDim = flow.nVelocities();
	unsigned int nScalars = flow.nScalars();
	unsigned int nFields = flow.nFields();
	std::vector<float> values(count * nFields);
	MPI_Recv(values.data(), count * nFields, MPI_FLOAT, neiRank, Tag_Ptcls, MPI_COMM_WORLD, &status);
	// populate the received data into particles
	PtclArr particles(count);
	for (unsigned int i = 0; i < count; ++i)
	{
		particles[i].id() = ids[i];
		for (unsigned int j = 0; j < nFields; ++j)
			particles[i][j] = values[i * nFields + j];
#ifdef DEBUG_TEXT
		std::cout << "Recv: Proc " << DomainInfo::myRank() << ": " << particles[i].coord() << std::endl;
#endif
	}
	return particles;
}