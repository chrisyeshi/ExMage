//
//
// PtclSync
//
// This class handles the communication among sub volumes
// by performing a 2 step synchronize of the particles.
// * Step 1: Send the current and next particles to the corresponding neighbors.
// * Step 2: Receive the next particle properties from the neighbors.
//
//

#ifndef __PtclSync_h__
#define __PtclSync_h__

#include <vector>
#include <map>

#include "Vector.h"
#include "Particle.h"
#include "ConfigReader.h"

class PtclSync
{
private:
	const static int vDim = 3;
	typedef std::vector<Particle<> > PtclArr;
	typedef std::map<Vector<vDim, int>, PtclArr> PtclMap;
	const static int Tag_Count = 1;
	const static int Tag_Ids = 2;
	const static int Tag_Ptcls = 3;

public:
	PtclSync();

	void scatter(const PtclArr& currBorder, const PtclArr& nextBorder, const PtclArr& curr, const PtclArr& next, const VectorField<>& flow);
    PtclArr getCurrOutBorder() const;
    PtclArr getNextOutBorder() const;
	PtclArr getCurrOut() const;
	PtclArr getNextOut() const;
	PtclArr getCurrInc() const;
	PtclArr getNextInc() const;

protected:
    void mapBorder(const PtclArr& currBorder, const PtclArr& nextBorder);
	void mapPtcls(const PtclArr& curr, const PtclArr& next);
	void scatter();
	void send(const Vector<vDim, int> neighbor3, const PtclArr& ptcls) const;
	PtclArr recv(const Vector<vDim, int> neighbor3) const;

private:
    PtclMap currInBorder;
    PtclMap nextInBorder;
    PtclMap currOutBorder;
    PtclMap nextOutBorder;
	PtclMap currOut;
	PtclMap nextOut;
	PtclMap currInc;
	PtclMap nextInc;
	VectorField<> flow;
};

#endif //__PtclSync_h__