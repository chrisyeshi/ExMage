#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <string>

#include "mpi.h"

#include "ParticleAdvector.h"
#include "ConfigReader.h"
#include "VectorFieldReader.h"
#include "CoreTube.h"
#include "Frame.h"
#include "mkpath.h"
#include "DomainUtility.h"
#include "Explorable.h"
#include "ProcIndex.h"

CoreTube coretube;
// std::vector<tube::Particle> translate2tubeparticle(const std::vector<Particle<> >& particles);

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    time_t start, end;
    double dif;
    time(&start);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // read vector field
    VectorFieldReader reader;
    std::vector<float*> fields;
    assert(reader.read());
    fields = reader.getFields();

    //
    //
    // This is the expected initialization code.
    // 1. Initialize our class
    // 2. Set properties that should be obtained from the simulation codes
    // 3. Otherwise, the properties are set by the config file
    //
    //
    Explorable explorable;
    explorable.setConfigFile("configure.json"); // If not set, it's default to be configure.json

    //
    //
    // Here is the fake Simulation main loop
    // Currently, I am still reading the timestep from the config file.
    // Later on, I should remove that because the simulation should be taking care of that.
    //
    //
    ConfigReader& config = ConfigReader::getInstance();
    std::vector<int> range = config.GetTimeStepRange();
    for (int timestep = range[0]; timestep <= range[1]; ++timestep)
    {
        //
        //
        // Inside the Simulate loop.
        //
        //
        explorable.update(fields);
    }

    //
    //
    // After the Simulation is done, we need the simulation to initiate the output command.
    //
    //
    explorable.output();

    DomainUtility domainUtil;
    std::vector<Vector<> > bounds = domainUtil.getBounds();
    std::cout << "Proc " << rank << " Bounds: [" << bounds[0].x() << ", " << bounds[1].x() << ", " << bounds[0].y() << ", " << bounds[1].y() << ", " << bounds[0].z() << ", " << bounds[1].z() << "]" << std::endl;

/*
    // choose configure file
    ConfigReader::setFile("configure.json");
    // particle tracer
    ParticleAdvector sim;
    // tube generator
    coretube.Initialize();

    // trace particles
    std::vector<int> range = config.GetTimeStepRange();
    for (int timestep = range[0]; timestep <= range[1]; ++timestep)
    {
        sim.trace(fields);
        coretube.GenerateTubes(sim.prevParticles(), sim.nextParticles());
    }

    // output and finalize
    coretube.Output();
*/



    std::cout << "finalize" << std::endl;
    time(&end);
    dif = difftime(end, start);
    std::cout << "Time: " << dif << "\n";
    MPI_Finalize();
    return 0;
}