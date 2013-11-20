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
    DomainUtility domain;
    double extent[6];
    extent[0] = domain.getBounds()[0].x();
    extent[1] = domain.getBounds()[1].x();
    extent[2] = domain.getBounds()[0].y();
    extent[3] = domain.getBounds()[1].y();
    extent[4] = domain.getBounds()[0].z();
    extent[5] = domain.getBounds()[1].z();
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