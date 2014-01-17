#include <map>

#include "DomainUtility.h"
#include "cassert"
#include "mpi.h"

DomainUtility::DomainUtility()
{
}

bool DomainUtility::inBounds(const Vector<>& coord) const
{
	for (int i = 0; i < vDim; ++i)
		if (coord[i] < bounds()[0][i] || coord[i] >= bounds()[1][i])
			return false;
	return true;
}

void DomainUtility::scatter(const PtclArr& curr, const PtclArr& next, const VectorField<>& flow)
{
	mapPtcls(curr, next);
	this->flow = flow;
	this->scatter();
}

DomainUtility::PtclArr DomainUtility::getCurrInc() const
{
	PtclArr ret;
	PtclMap::const_iterator itr = currInc.begin();
	for (; itr != currInc.end(); ++itr)
		ret.insert(ret.end(), itr->second.begin(), itr->second.end());
	return ret;
}

DomainUtility::PtclArr DomainUtility::getNextInc() const
{
	PtclArr ret;
	PtclMap::const_iterator itr = nextInc.begin();
	for (; itr != nextInc.end(); ++itr)
		ret.insert(ret.end(), itr->second.begin(), itr->second.end());
	return ret;
}

void DomainUtility::mapPtcls(const PtclArr& curr, const PtclArr& next)
{
	currOut.clear();
	nextOut.clear();
	for (unsigned int i = 0; i < next.size(); ++i)
	{
		Vector<vDim, int> neighbor3(0, 0, 0);
		for (int j = 0; j < vDim; ++j)
		{
			if (next[i].coord()[j] < bounds()[0][j])
				neighbor3[j] = -1;
			if (next[i].coord()[j] >= bounds()[1][j])
				neighbor3[j] = 1;
		}
		assert((neighbor3 != Vector<vDim, int>(0, 0, 0)));
		currOut[neighbor3].push_back(curr[i]);
		nextOut[neighbor3].push_back(next[i]);
	}
}

void DomainUtility::scatter()
{
	currInc.clear();
	nextInc.clear();
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
		PtclMap::iterator itr = currInc.find(neighbor3);
		nextInc[neighbor3] = recv(neighbor3);
		for (unsigned int i = 0; i < nextInc[neighbor3].size(); ++i)
			nextInc[neighbor3][i].scalars() = flow.getScalars(nextInc[neighbor3][i].coord() - bounds()[0]);
		// assert(currInc[neighbor3].size() == nextInc[neighbor3].size());
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

void DomainUtility::send(const Vector<vDim, int> neighbor3, const PtclArr& ptcls) const
{
	// don't send stuff to myself
	if (neighbor3 == Vector<vDim, int>(0, 0, 0))
		return;
	Vector<vDim, int> neiRank3 = myRank3() + neighbor3;
	if (!inVolume(neiRank3))
	{
		// TODO: add codes to fill scalars for the nextOut particles
		return;
	}
	// send particle count to neighbor
	int neiRank = toRank(neiRank3);
	unsigned int count = ptcls.size();
	MPI_Send(&count, 1, MPI_INT, neiRank, Tag_Count, MPI_COMM_WORLD);
	// if no particle, then stop
	if (count == 0)
		return;
	std::cout << "Send: Proc " << myRank() << " is sending " << count << " particles to Proc" << neiRank << std::endl;
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
		std::cout << "Send: Proc " << myRank() << ": " << ptcls[i].coord() << std::endl;
		for (unsigned int j = 0; j < vDim; ++j)
			values[i * nFields + j] = ptcls[i].coord()[j];
		for (unsigned int j = 0; j < nScalars; ++j)
			values[i * nFields + vDim + j] = ptcls[i].scalar(j);
	}
	MPI_Send(values.data(), count * nFields, MPI_FLOAT, neiRank, Tag_Ptcls, MPI_COMM_WORLD);
}

DomainUtility::PtclArr DomainUtility::recv(const Vector<vDim, int> neighbor3) const
{
	// not receiving any stuff from myself...
	if (neighbor3 == Vector<vDim, int>(0, 0, 0))
		return PtclArr();
	Vector<vDim, int> neiRank3 = myRank3() + neighbor3;
	if (!inVolume(neiRank3))
		return PtclArr();
	// receive particle count from neighbor
	int neiRank = toRank(neiRank3);
	MPI_Status status;
	unsigned int count;
	MPI_Recv(&count, 1, MPI_UNSIGNED, neiRank, Tag_Count, MPI_COMM_WORLD, &status);
	// if no particle, then stop
	if (count == 0)
		return PtclArr();
	std::cout << "Proc " << myRank() << " bounds: " << bounds()[0] << ", " << bounds()[1] << std::endl;
	std::cout << "Recv: Proc " << myRank() << " is receiving " << count << " particles from Proc" << neiRank << std::endl;
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
		std::cout << "Recv: Proc " << myRank() << ": " << particles[i].coord() << std::endl;
		// assert(inBounds(particles[i].coord()));
	}
	return particles;
}

int DomainUtility::myRank() const
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank;
}

Vector<DomainUtility::vDim, int> DomainUtility::myRank3() const
{
	return toRank3(myRank());
}

int DomainUtility::toRank(const Vector<vDim, int>& rank3) const
{
	return rank3[0] + nRegions3()[0] * rank3[1] + nRegions3()[0] * nRegions3()[1] * rank3[2];
}

Vector<DomainUtility::vDim, int> DomainUtility::toRank3(int rank) const
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

std::vector<Vector<> > DomainUtility::bounds() const
{
	std::vector<Vector<> > ret(2);
	for (int i = 0; i < vDim; ++i)
	{
		ret[0][i] = volDim()[i] / nRegions3()[i] * (myRank3()[i] + 0);
		ret[1][i] = volDim()[i] / nRegions3()[i] * (myRank3()[i] + 1);
	}
	return ret;
}

bool DomainUtility::inVolume(const Vector<vDim, int>& rank3) const
{
	for (int i = 0; i < vDim; ++i)
		if (rank3[i] < 0 || rank3[i] >= nRegions3()[i])
			return false;
	return true;
}