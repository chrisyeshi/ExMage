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

    // trace particles
    Simulator sim;
    std::vector<int> range = config.GetTimeStepRange();
    for (int timestep = range[0]; timestep <= range[1]; ++timestep)
    {
        sim.trace(fields);
    }

    // output and finalize
    sim.output();
    std::cout << "finalize" << std::endl;
    time(&end);
    dif = difftime(end, start);
    std::cout << "Time: " << dif << "\n";
    MPI_Finalize();
    return 0;
}
