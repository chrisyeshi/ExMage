#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <ctime>

#include "mpi.h"

#include "Simulator.h"
#include "ConfigReader.h"

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
  int TOTAL_SIZE_X = config.GetTotalSize()[0];
  int TOTAL_SIZE_Y = config.GetTotalSize()[1];
  int TOTAL_SIZE_Z = config.GetTotalSize()[2];
  int REGION_COUNT_X = config.GetRegionCount()[0];
  int REGION_COUNT_Y = config.GetRegionCount()[1];
  int REGION_COUNT_Z = config.GetRegionCount()[2];
//  int TOTAL_REGION_COUNT = REGION_COUNT_X * REGION_COUNT_Y * REGION_COUNT_Z;
  int REGION_PARTICLE_COUNT = config.GetRegionParticleCount();
//  int TOTAL_PARTICLE_COUNT = REGION_PARTICLE_COUNT * TOTAL_REGION_COUNT;
  const std::string& ROOT_DIRECTORY = config.GetReadRoot();
  const std::string& OUT_ROOT_DIRECTORY = config.GetOutRoot();
  int TIMESTEP_START = config.GetTimeStepRange()[0];
  int TIMESTEP_END = config.GetTimeStepRange()[1];
  float VELOCITY = config.GetVelocity();

  // region index
  int region_index[3];
  int index = rank;
  region_index[0] = index % REGION_COUNT_X;
  index /= REGION_COUNT_X;
  region_index[1] = index % REGION_COUNT_Y;
  index /= REGION_COUNT_Y;
  region_index[2] = index;
//  std::cout << "Rank: " << rank << " ==> ";
//  std::cout << "Region Index: " << region_index[0]
//                        << ", " << region_index[1]
//                        << ", " << region_index[2] << std::endl;
  assert(region_index[2] < REGION_COUNT_Z);
  // region_bound
  float region_bound[6]; // minx, maxx, miny, maxy, minz, maxz
  region_bound[0] = TOTAL_SIZE_X / REGION_COUNT_X * (region_index[0] + 0);
  region_bound[1] = TOTAL_SIZE_X / REGION_COUNT_X * (region_index[0] + 1);
  region_bound[2] = TOTAL_SIZE_Y / REGION_COUNT_Y * (region_index[1] + 0);
  region_bound[3] = TOTAL_SIZE_Y / REGION_COUNT_Y * (region_index[1] + 1);
  region_bound[4] = TOTAL_SIZE_Z / REGION_COUNT_Z * (region_index[2] + 0);
  region_bound[5] = TOTAL_SIZE_Z / REGION_COUNT_Z * (region_index[2] + 1);
  std::cout << "Rank: " << rank << " ==> Region Bound: ";
  for (int i = 0; i < 6; ++i)
  {
    std::cout << region_bound[i] << ", ";
  }
  std::cout << "\n";
  {
    // simulation
    Simulator sim;
    sim.loadConfig();
    sim.set_region_index(region_index);
    sim.set_region_bound(region_bound);

    sim.simulate();
  }

  std::cout << "finalize" << std::endl;
  time(&end);
  dif = difftime(end, start);
  std::cout << "Time: " << dif << "\n";
  MPI_Finalize();
  return 0;
}
