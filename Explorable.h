//
//
// The top level interface and controller of the whole library package.
//
//

#ifndef __EXPLORABLE_H__
#define __EXPLORABLE_H__

#include <string>
#include "ParticleAdvector.h"
#include "CoreTube.h"

class Explorable
{
public:
    Explorable();
    ~Explorable();

    void setConfigFile(const std::string& filename);
    void update(const std::vector<float*>& fields);
    void output();

protected:

private:
    ParticleAdvector advector;
    CoreTube tuber;
};

#endif //__EXPLORABLE_H__