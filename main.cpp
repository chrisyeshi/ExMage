#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>

#include "mpi.h"

#include "Simulator.h"

#define TOTAL_SIZE_X 600
#define TOTAL_SIZE_Y 600
#define TOTAL_SIZE_Z 600

#define REGION_COUNT_X 4
#define REGION_COUNT_Y 4
#define REGION_COUNT_Z 4
#define TOTAL_REGION_COUNT (REGION_COUNT_X * REGION_COUNT_Y * REGION_COUNT_Z)

#define TOTAL_PARTICLE_COUNT 12800
#define REGION_PARTICLE_COUNT (TOTAL_PARTICLE_COUNT / TOTAL_REGION_COUNT)

#define ROOT_DIRECTORY "/home/cluster/supernova_64/"
#define OUT_ROOT_DIRECTORY "/home/cluster/shared_space/Yucong/supernova/id/"

#define TIMESTEP_START 1403
#define TIMESTEP_END 1503

#define VELOCITY 8.0

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  srand(time(0) * rank);
  // region index
  int region_index[3];
  int index = rank;
  region_index[0] = index % REGION_COUNT_X;
  index /= REGION_COUNT_X;
  region_index[1] = index % REGION_COUNT_Y;
  index /= REGION_COUNT_Y;
  region_index[2] = index;
  std::cout << "Rank: " << rank << " ==> ";
  std::cout << "Region Index: " << region_index[0]
                        << ", " << region_index[1]
                        << ", " << region_index[2] << std::endl;
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
  std::cout << std::endl;
  // timestep range
  int timestep_range[2] = {TIMESTEP_START, TIMESTEP_END};
  // region_count
  int region_count[3] = {REGION_COUNT_X, REGION_COUNT_Y, REGION_COUNT_Z};
//  for (int i = 0; i < 19; ++i)
  {
    // particle count and out root directory
    int total_particle_count = 100 * 256;
    std::cout << "Generating Particle Count " << total_particle_count << std::endl;
    int region_particle_count = total_particle_count / TOTAL_REGION_COUNT;
    char out_root_directory[100];
    sprintf(out_root_directory, "/home/cluster/shared_space/Yucong/supernova/p%d/", total_particle_count);
    // simulation
    Simulator sim;
    sim.set_region_count(region_count);
    sim.set_region_index(region_index);
    sim.set_region_bound(region_bound);
    sim.set_root(ROOT_DIRECTORY);
    sim.set_out_root(out_root_directory);
    sim.set_timestep_range(timestep_range);
    sim.set_velocity(VELOCITY);
    sim.simulate(region_particle_count);
  }

  std::cout << "finalize" << std::endl;
  MPI_Finalize();
  return 0;
}
