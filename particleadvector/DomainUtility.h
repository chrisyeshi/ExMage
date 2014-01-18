//
//
// DomainUtility
//
// This class provides the domain information, such as bounds, volume size, number of sub volumes, etc.
// Also, it handles the communication among sub volumes by performing a 2 step synchronize of the particles.
// * Step 1: Send the current and next particles to the corresponding neighbors.
// * Step 2: Receive the next particle properties from the neighbors.
//
//

#ifndef __DomainUtility_h__
#define __DomainUtility_h__

#include <vector>
#include <map>

#include "Vector.h"
#include "Particle.h"
#include "ConfigReader.h"

class DomainUtility
{
private:
	const static int vDim = 3;
	typedef std::vector<Particle<> > PtclArr;
	typedef std::map<Vector<vDim, int>, PtclArr> PtclMap;
	const static int Tag_Count = 1;
	const static int Tag_Ids = 2;
	const static int Tag_Ptcls = 3;

public:
	DomainUtility();

	std::vector<Vector<> > getBounds() const { return bounds(); }
	bool inBounds(const Vector<>& coord) const;
	void scatter(const PtclArr& curr, const PtclArr& next, const VectorField<>& flow);
	PtclArr getCurrOut() const;
	PtclArr getNextOut() const;
	PtclArr getCurrInc() const;
	PtclArr getNextInc() const;

protected:
	void mapPtcls(const PtclArr& curr, const PtclArr& next);
	void scatter();
	void send(const Vector<vDim, int> neighbor3, const PtclArr& ptcls) const;
	PtclArr recv(const Vector<vDim, int> neighbor3) const;
	int myRank() const;
	Vector<vDim, int> myRank3() const;
	int toRank(const Vector<vDim, int>& rank3) const;
	Vector<vDim, int> toRank3(int rank) const;
	std::vector<Vector<> > bounds() const;
	Vector<> ranges() const { return bounds()[1] - bounds()[0]; }
	bool inVolume(const Vector<vDim, int>& rank3) const;

private:
	PtclMap currOut;
	PtclMap nextOut;
	PtclMap currInc;
	PtclMap nextInc;
	VectorField<> flow;

	ConfigReader& config() const { return ConfigReader::getInstance(); }
	std::vector<int> volDim() const { return config().GetTotalSize(); }
	std::vector<int> nRegions3() const { return config().GetRegionCount(); }
};

#endif //__DomainUtility_h__