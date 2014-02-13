#ifndef __DomainInfo_h__
#define __DomainInfo_h__

#include <vector>
#include "ConfigReader.h"
#include "Vector.h"

class DomainInfo
{
public:
    const static int vDim = 3;
    static int myRank();
    static Vector<vDim, int> myRank3();
    static int toRank(const Vector<vDim, int>& rank3);
    static Vector<vDim, int> toRank3(const int& rank);

    static std::vector<Vector<> > bounds(); // my bounds
    static std::vector<Vector<> > bounds(const int& rank);
    static std::vector<Vector<> > bounds(const Vector<vDim, int>& rank3);

    static Vector<vDim> ranges(); // my ranges
    static Vector<vDim> ranges(const int& rank);
    static Vector<vDim> ranges(const Vector<vDim, int>& rank3);

    static bool inBounds(const Vector<vDim>& coord); // test against my bounds
    static bool inBounds(const Vector<vDim>& coord, const int& rank);
    static bool inBounds(const Vector<vDim>& coord, const Vector<vDim, int>& rank3);

    static bool inVolume(const Vector<vDim, int>& rank3);

protected:

private:
    static ConfigReader& config() { return ConfigReader::getInstance(); }
    static std::vector<int> volDim() { return config().get("domain.volume").asArray<double, int>(); }
    static std::vector<int> nRegions3() { return config().get("domain.count").asArray<double, int>(); }

};

#endif //__DomainInfo_h__