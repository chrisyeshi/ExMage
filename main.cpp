#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <string>

#include "mpi.h"
#include "VectorFieldReader.h"
#include "Explorable.h"

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
    std::vector<int> range = config.get("input.time").asArray<double, int>();
    for (int timestep = range[0]; timestep <= range[1]; ++timestep)
    {
        //
        //
        // Inside the Simulate loop.
        //
        // The first 3 fields are the velocity fields, the 4th/last field is
        // the scalar field that we do visualization on.
        //
        explorable.update(fields);
    }

    //
    //
    // After the Simulation is done, we need the simulation to initiate the output command.
    //
    //
    explorable.output();

    time(&end);
    dif = difftime(end, start);
    MPI_Finalize();
    return 0;
}