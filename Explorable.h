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

    //
    //
    // Sets the configuration file: (If not set, it defaults to "configure.json")
    // * The configuration file format is specified in the documentation.
    // * As sample configuration file is provided "configure.json.sample"
    //
    //
    void setConfigFile(const std::string& filename);

    //
    //
    // Function to be called in the simulation main loop:
    // * The first 3 fields are the veolocity x, y, and z.
    // * the 4th field is the scalar field we are visualizing on.
    // (This version we are only supporting 1 scalar field)
    //
    //
    void update(const std::vector<float*>& fields);

    //
    //
    // After the simulation is done, use this function to gather all the framebuffers
    // from different nodes, and then output to a single explorable image.
    //
    //
    void output();

protected:

private:
    ParticleAdvector advector;
    CoreTube tuber;
};

#endif //__EXPLORABLE_H__