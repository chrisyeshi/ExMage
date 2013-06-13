#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <string>

#include "mpi.h"

#include "Simulator.h"
#include "ConfigReader.h"
#include "VectorFieldReader.h"
#include "CoreTube.h"
#include "Frame.h"
#include "mkpath.h"

CoreTube coretube;
std::vector<tube::Particle> translate2tubeparticle(const std::vector<Particle>& particles);
void output();

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    time_t start, end;
    double dif;
    time(&start);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    srand(time(0) * rank);

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
    Simulator sim;

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
        coretube.GenerateTubes(
                translate2tubeparticle(sim.prevParticles()),
                translate2tubeparticle(sim.nextParticles()));
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

std::vector<tube::Particle> translate2tubeparticle(const std::vector<Particle>& particles)
{
    std::vector<tube::Particle> ret(particles.size());
    for (unsigned int i = 0; i < particles.size(); ++i)
    {
        ret[i].x = particles[i].position[0];
        ret[i].y = particles[i].position[1];
        ret[i].z = particles[i].position[2];
        ret[i].pd = particles[i].scalars[0];
        ret[i].id = particles[i].tube_id;
    }
    return ret;
}
/*
void output()
{
    ConfigReader& config = ConfigReader::getInstance();
    // output frames
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::cout << "Proc: " << rank << " Progress: Saving..." << std::endl;
    for (int i = 0; i < coretube.GetCameraCount(); ++i)
    {
        Frame* sum = coretube.GetFrame(i);
        char proc_index_string[10];
        sprintf(proc_index_string, "%02d", rank);
        char cam_index_string[10];
        sprintf(cam_index_string, "%02d", i);

        char pcs[100];
        int resolution[2];
        sum->GetSize(resolution);
        sprintf(pcs, "n%d_p%d_r%d_t%d",
                config.GetRegionCount()[0],
                config.GetRegionParticleCount(),
                resolution[0],
                config.GetTimeStepRange()[1] - config.GetTimeStepRange()[0]);

        std::string dir = config.GetOutRoot() + "/" + pcs
                        + std::string("/cam") + cam_index_string;
        std::string spt_path = dir + "/output_proc" + proc_index_string;
        mkpath(dir.c_str(), 0777);
        mkpath(dir.c_str(), 0777);
        mkpath(dir.c_str(), 0777);
        sum->SetFileName(spt_path.c_str());
        sum->Write();
    }
    // output times
    Frame* sum = coretube.GetFrame(0);
    char pcs[100];
    int resolution[2];
    sum->GetSize(resolution);
    sprintf(pcs, "n%d_p%d_r%d_t%d",
            config.GetRegionCount()[0],
            config.GetRegionParticleCount(),
            resolution[0],
            config.GetTimeStepRange()[1] - config.GetTimeStepRange()[0]);

    std::string dir = config.GetOutRoot() + "/" + pcs + "/time";
    char global_index_string[10];
    sprintf(global_index_string, "%02d", rank);
    std::string time_path = dir + "/proc" + global_index_string + ".txt";
    mkpath(dir.c_str(), 0777);
    mkpath(dir.c_str(), 0777);
    mkpath(dir.c_str(), 0777);
    std::ofstream tout(time_path.c_str());
    for (unsigned int i = 0; i < times.size(); ++i)
    {
        tout << times[i] << ",";
    }
}
*/
