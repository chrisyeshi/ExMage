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
    // srand(time(0) * rank);

    // choose configure file
    ConfigReader::setFile("configure.json");

    // read configure file
    ConfigReader& config = ConfigReader::getInstance();

    // read vector field
    VectorFieldReader reader;
    std::vector<float*> fields;
    assert(reader.read());
    fields = reader.getFields();

    // particle tracer
    ParticleAdvector sim;

    // tube generator
    coretube.Initialize();
    coretube.SetCameras(config.GetCameras());
    coretube.SetLightPosition(config.GetLightPosition());
    double extent[6];
    for (int i = 0; i < 6; ++i)
        extent[i] = sim.region_bound()[i];
    coretube.SetExtent(extent);

    // trace particles
    std::vector<int> range = config.GetTimeStepRange();
    for (int timestep = range[0]; timestep <= range[1]; ++timestep)
    {
        sim.trace(fields);
        coretube.GenerateTubes(sim.prevParticles(), sim.nextParticles());
    }

    // output and finalize
    coretube.Output();
    std::cout << "finalize" << std::endl;
    time(&end);
    dif = difftime(end, start);
    std::cout << "Time: " << dif << "\n";
    MPI_Finalize();
    return 0;
}