#include "Simulator.h"

#include <map>
#include <fstream>
#include <iostream>
#include <cassert>
#include <sys/stat.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <ctime>

#include "mpi.h"
#include "Particle.h"
#include "Frame.h"

#define TAG_TIMESTEP 1
#define TAG_TIMESTEP_DATA 2

///////////////////////////////////////////////////////////////////////////////////
//
//
//
// Make Directory
//
//
//
///////////////////////////////////////////////////////////////////////////////////

typedef struct stat Stat;

static int do_mkdir(const char *path, mode_t mode)
{
    Stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist */
        if (mkdir(path, mode) != 0)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}

/**
 * ** mkpath - ensure all directories in path exist
 * ** Algorithm takes the pessimistic view and works top-down to ensure
 * ** each directory in path exists, rather than optimistically creating
 * ** the last element and working backwards.
 * */
int mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    free(copypath);
    return (status);
}

////////////////////////////////////////////////////////////////////////////////
//
//
//
// Public
//
//
//
////////////////////////////////////////////////////////////////////////////////

Simulator::Simulator()
{
  current_timestep_ = 0;

  for (int i = 0; i < 6; ++i)
    flow_field_[i] = NULL;

  in_attributes_.resize(6);
  in_attributes_[0] = "xvelocity";
  in_attributes_[1] = "yvelocity";
  in_attributes_[2] = "zvelocity";
  in_attributes_[3] = "entropy";
  in_attributes_[4] = "density";
  in_attributes_[5] = "temperature";

  out_attributes_.resize(6);
  out_attributes_[0] = "x";
  out_attributes_[1] = "y";
  out_attributes_[2] = "z";
  out_attributes_[3] = "entropy";
  out_attributes_[4] = "density";
  out_attributes_[5] = "temperature";

  config_reader_.SetFileName("configure.txt");
  assert(config_reader_.Read());
}

Simulator::~Simulator()
{
}


void Simulator::simulate(int particle_count)
{
  current_timestep_ = timestep_range_[0];
  read(current_timestep_);
  initializeParticles(particle_count);

  // use the begin timestep flow field to do streamline
  assert(read(timestep_range_[0]));

  out_timestep_ = 0;
  for (current_timestep_ = timestep_range_[0] + 1;
       current_timestep_ <= timestep_range_[1]; ++current_timestep_)
  {
    // read each time would result in a pathline
//    if (!read(current_timestep_))
//      continue;
    std::cout << "Timestep: " << out_timestep_ << std::endl;
    ++out_timestep_;
    traceParticles();
    communicateWithNeighbors();
    writeToFile();
    // clean up
//    for (int i = 0; i < 6; ++i)
//    {
//      delete [] flow_field_[i];
//      flow_field_[i] = NULL;
//    }
  }
  // output frames
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  std::cout << "Proc: " << rank << " Progress: Saving..." << std::endl;
  for (int i = 0; i < coretube_.GetCameraCount(); ++i)
  {
    Frame* sum = coretube_.GetFrame(i);
    char proc_index_string[10];
    sprintf(proc_index_string, "%02d", rank);
    char cam_index_string[10];
    sprintf(cam_index_string, "%02d", i);
    char particle_count_string[100];
    int resolution[2];
    sum->GetSize(resolution);
    sprintf(particle_count_string, "n%d_p%d_r%d_t%d", region_count_[0], particle_count, resolution[0], timestep_range_[1] - timestep_range_[0]);
    std::string dir = out_root_ + "/" + particle_count_string + std::string("/cam") + cam_index_string;
    std::string spt_path = dir + "/output_proc" + proc_index_string;
    mkpath(dir.c_str(), 0777);
    mkpath(dir.c_str(), 0777);
    mkpath(dir.c_str(), 0777);
    sum->SetFileName(spt_path.c_str());
    sum->Write();
  }
  // output times
  char particle_count_string[100];
  int resolution[2];
  coretube_.GetFrame(0)->GetSize(resolution);
  sprintf(particle_count_string, "n%d_p%d_r%d_t%d", region_count_[0], particle_count, resolution[0], timestep_range_[1] - timestep_range_[0]);
  char proc_index_string[10];
  sprintf(proc_index_string, "%02d", rank);
  std::string dir = out_root_ + "/" + particle_count_string + "/time";
  std::string time_path = dir + "/proc" + proc_index_string + ".txt";
  mkpath(dir.c_str(), 0777);
  mkpath(dir.c_str(), 0777);
  mkpath(dir.c_str(), 0777);
  std::ofstream tout(time_path.c_str());
  for (unsigned int i = 0; i < times_.size(); ++i)
  {
    tout << times_[i] << ",";
  }
  // clean up
  for (int i = 0; i < 6; ++i)
  {
    delete [] flow_field_[i];
    flow_field_[i] = NULL;
  }
}

void Simulator::set_global_size(int global_size[3])
{
  for (int i = 0; i < 3; ++i)
    global_size_[i] = global_size[i];
}

void Simulator::set_region_count(int region_count[3])
{
  region_count_[0] = region_count[0];
  region_count_[1] = region_count[1];
  region_count_[2] = region_count[2];
}

void Simulator::set_region_index(int region_index[3])
{
  region_index_[0] = region_index[0];
  region_index_[1] = region_index[1];
  region_index_[2] = region_index[2];
}

void Simulator::set_region_bound(float region_bound[6])
{
  for (int i = 0; i < 6; ++i)
    region_bound_[i] = region_bound[i];
}

void Simulator::set_root(const std::string& root)
{
  root_ = root;
}

void Simulator::set_out_root(const std::string& out_root)
{
  out_root_ = out_root;
}

void Simulator::set_timestep_range(int timestep_range[2])
{
  timestep_range_[0] = timestep_range[0];
  timestep_range_[1] = timestep_range[1];
}

void Simulator::set_velocity(float velocity)
{
  velocity_ = velocity;
}

////////////////////////////////////////////////////////////////////////////////
//
//
//
// Protected
//
//
//
////////////////////////////////////////////////////////////////////////////////

void Simulator::initializeParticles(int particle_count)
{
  particles_current_.resize(particle_count);
  for (int j = 0; j < particle_count; ++j)
  {
    float range[3] = {region_bound_[1] - region_bound_[0],
                      region_bound_[3] - region_bound_[2],
                      region_bound_[5] - region_bound_[4]};
    for (int i = 0; i < 3; ++i)
    {
      particles_current_[j].position[i] = float(rand() % int(range[i] * 1000)) / 1000.0 + region_bound_[i * 2];
    }
    // fill scalars
    fillParticleScalars(&particles_current_[j]);
    // id
    particles_current_[j].tube_id = j;
  }
}

int Simulator::regionIndexToRank(const int region_index[3]) const
{
  return region_index[0]
       + region_count_[0] * region_index[1]
       + region_count_[0] * region_count_[1] * region_index[2];
}

void Simulator::rankToRegionIndex(int rank, int region_index[3]) const
{
  int index = rank;
  region_index[0] = index % region_count_[0];
  index /= region_count_[0];
  region_index[1] = index % region_count_[1];
  index /= region_count_[1];
  region_index[2] = index;
  assert(region_index[2] < region_count_[2]);
}

Particle Simulator::findBoundaryParticle(const Particle& curr, const Particle& next) const
{
//  for (int i = 0; i < 3; ++i)
//    if (particle.position[i] < region_bound_[2 * i]
//     || particle.position[i] >= region_bound_[2 * i + 1] + 1.0)
//      return false;
//  return true;
  float vector_full[3];
  for (int i = 0; i < 3; ++i)
    vector_full[i] = next.position[i] - curr.position[i];
  float ratio = 1.0;
  for (int i = 0; i < 3; ++i)
  {
    float r = 1.0;
    if (vector_full[i] < 0)
    {
      if (next.position[i] < region_bound_[2 * i])
        r = (region_bound_[2 * i] - curr.position[i]) / (next.position[i] - curr.position[i]);
    } else if (vector_full[i] > 0)
    {
      if (next.position[i] >= region_bound_[2 * i + 1] + 1.0)
        r = (region_bound_[2 * i + 1] + 1.0 - curr.position[i]) / (next.position[i] - curr.position[i]);
    }
    ratio = std::min(r, ratio);
  }
  Particle ret;
  for (int i = 0; i < 3; ++i)
    ret.position[i] = curr.position[i] + vector_full[i] * ratio;
  ret.tube_id = curr.tube_id;
  return ret;
}

void Simulator::traceParticles()
{
  leaving_particles_current_.clear();
  leaving_particles_next_.clear();
  std::vector<Particle> particles_current;
  std::vector<Particle> particles_next;
  for (unsigned int i = 0; i < particles_current_.size(); ++i)
  {
    Particle next_particle = traceParticle(particles_current_[i]);
    if (isParticleInside(next_particle))
    {
      particles_current.push_back(particles_current_[i]);
      fillParticleScalars(&next_particle);
      particles_next.push_back(next_particle);
    } else
    {
      Particle current_particle = particles_current_[i];
//      Particle boundary_particle = findBoundaryParticle(current_particle, next_particle);
//      fillParticleScalars(&boundary_particle);
//      particles_current.push_back(current_particle);
//      particles_next.push_back(boundary_particle);
      leaving_particles_current_.push_back(current_particle);
      leaving_particles_next_.push_back(next_particle);
    }
  }
  particles_current_ = particles_current;
  particles_next_ = particles_next;
}

Particle Simulator::traceParticle(const Particle& particle) const
{
  float velocity3[3];
  getParticleVelocity(particle, velocity3);
//  std::cout << "Velocity: " << velocity3[0] << ", " << velocity3[1] << ", " << velocity3[2] << std::endl;
  Particle ret;
  ret.position[0] = particle.position[0] + velocity3[0] * velocity_;
  ret.position[1] = particle.position[1] + velocity3[1] * velocity_;
  ret.position[2] = particle.position[2] + velocity3[2] * velocity_;
  ret.tube_id = particle.tube_id;
  return ret;
}

void Simulator::getParticleVelocity(const Particle& particle, float velocity3[3]) const
{
  float region_size[3] = {region_bound_[1] - region_bound_[0],
                          region_bound_[3] - region_bound_[2],
                          region_bound_[5] - region_bound_[4]};
  int lower_bound[3];
  lower_bound[0] = int(particle.position[0] - region_bound_[0]);
  lower_bound[1] = int(particle.position[1] - region_bound_[2]);
  lower_bound[2] = int(particle.position[2] - region_bound_[4]);
  lower_bound[0] = std::min(int(region_bound_[1] - region_bound_[0]) - 2, lower_bound[0]);
  lower_bound[1] = std::min(int(region_bound_[3] - region_bound_[2]) - 2, lower_bound[1]);
  lower_bound[2] = std::min(int(region_bound_[5] - region_bound_[4]) - 2, lower_bound[2]);
  float ratio_xyz[3] = {particle.position[0] - region_bound_[0] - float(lower_bound[0]),
                        particle.position[1] - region_bound_[2] - float(lower_bound[1]),
                        particle.position[2] - region_bound_[4] - float(lower_bound[2])};
  float ratio_000 = ratio_xyz[0] * ratio_xyz[1] * ratio_xyz[2];
  float ratio_001 = ratio_xyz[0] * ratio_xyz[1] * (1.0 - ratio_xyz[2]);
  float ratio_010 = ratio_xyz[0] * (1.0 - ratio_xyz[1]) * ratio_xyz[2];
  float ratio_011 = ratio_xyz[0] * (1.0 - ratio_xyz[1]) * (1.0 - ratio_xyz[2]);
  float ratio_100 = (1.0 - ratio_xyz[0]) * ratio_xyz[1] * ratio_xyz[2];
  float ratio_101 = (1.0 - ratio_xyz[0]) * ratio_xyz[1] * (1.0 - ratio_xyz[2]);
  float ratio_110 = (1.0 - ratio_xyz[0]) * (1.0 - ratio_xyz[1]) * ratio_xyz[2];
  float ratio_111 = (1.0 - ratio_xyz[0]) * (1.0 - ratio_xyz[1]) * (1.0 - ratio_xyz[2]);
  int index_000 = lower_bound[0] + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_001 = lower_bound[0] + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  int index_010 = lower_bound[0] + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_011 = lower_bound[0] + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  int index_100 = (lower_bound[0] + 1) + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_101 = (lower_bound[0] + 1) + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  int index_110 = (lower_bound[0] + 1) + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_111 = (lower_bound[0] + 1) + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  velocity3[0] = flow_field_[0][index_000] * ratio_111 + flow_field_[0][index_001] * ratio_110
               + flow_field_[0][index_010] * ratio_101 + flow_field_[0][index_011] * ratio_100
               + flow_field_[0][index_100] * ratio_011 + flow_field_[0][index_101] * ratio_010
               + flow_field_[0][index_110] * ratio_001 + flow_field_[0][index_111] * ratio_000;
  velocity3[1] = flow_field_[1][index_000] * ratio_111 + flow_field_[1][index_001] * ratio_110
               + flow_field_[1][index_010] * ratio_101 + flow_field_[1][index_011] * ratio_100
               + flow_field_[1][index_100] * ratio_011 + flow_field_[1][index_101] * ratio_010
               + flow_field_[1][index_110] * ratio_001 + flow_field_[1][index_111] * ratio_000;
  velocity3[2] = flow_field_[2][index_000] * ratio_111 + flow_field_[2][index_001] * ratio_110
               + flow_field_[2][index_010] * ratio_101 + flow_field_[2][index_011] * ratio_100
               + flow_field_[2][index_100] * ratio_011 + flow_field_[2][index_101] * ratio_010
               + flow_field_[2][index_110] * ratio_001 + flow_field_[2][index_111] * ratio_000;
/*
  float region_size[3] = {region_bound_[1] - region_bound_[0],
                          region_bound_[3] - region_bound_[2],
                          region_bound_[5] - region_bound_[4]};
  float SIZE_X = region_size[0];
  float SIZE_Y = region_size[1];
  float SIZE_Z = region_size[2];
  int floor[3];
  floor[0] = int(particle.position[0] - region_bound_[0]);
  floor[1] = int(particle.position[1] - region_bound_[2]);
  floor[2] = int(particle.position[2] - region_bound_[4]);
  floor[0] = std::min(int(region_bound_[1] - region_bound_[0]) - 2, floor[0]);
  floor[1] = std::min(int(region_bound_[3] - region_bound_[2]) - 2, floor[1]);
  floor[2] = std::min(int(region_bound_[5] - region_bound_[4]) - 2, floor[2]);
  float ratio[3];
  for (int i = 0; i < 3; ++i)
    ratio[i] = particle.position[i] - float(floor[i]);
  for (int i = 0; i < 3; ++i)
  {
    // z
    int index[8];
    index[0] = floor[2] * SIZE_X * SIZE_Y + floor[1] * SIZE_X + floor[0];
    index[1] = floor[2] * SIZE_X * SIZE_Y + floor[1] * SIZE_X + floor[0] + 1;
    index[2] = floor[2] * SIZE_X * SIZE_Y + (floor[1] + 1) * SIZE_X + floor[0];
    index[3] = floor[2] * SIZE_X * SIZE_Y + (floor[1] + 1) * SIZE_X + floor[0] + 1;
    index[4] = (floor[2] + 1) * SIZE_X * SIZE_Y + floor[1] * SIZE_X + floor[0];
    index[5] = (floor[2] + 1) * SIZE_X * SIZE_Y + floor[1] * SIZE_X + floor[0] + 1;
    index[6] = (floor[2] + 1) * SIZE_X * SIZE_Y + (floor[1] + 1) * SIZE_X + floor[0];
    index[7] = (floor[2] + 1) * SIZE_X * SIZE_Y + (floor[1] + 1) * SIZE_X + floor[0] + 1;
    float xyz[8];
    for (int j = 0; j < 8; ++j)
      xyz[j] = flow_field_[i][index[j]];
    float xy[4];
    xy[0] = xyz[0] * (1 - ratio[2]) + xyz[4] * ratio[2];
    xy[1] = xyz[1] * (1 - ratio[2]) + xyz[5] * ratio[2];
    xy[2] = xyz[2] * (1 - ratio[2]) + xyz[6] * ratio[2];
    xy[3] = xyz[3] * (1 - ratio[2]) + xyz[7] * ratio[2];
    // y
    float x[2];
    x[0] = xy[0] * (1 - ratio[1]) + xy[2] * ratio[1];
    x[1] = xy[1] * (1 - ratio[1]) + xy[3] * ratio[1];
    // x
    velocity3[i] = x[0] * (1 - ratio[0]) + x[1] * ratio[0];
  }
*/
}

void Simulator::fillParticleScalars(Particle* particle) const
{
  float region_size[3] = {region_bound_[1] - region_bound_[0],
                          region_bound_[3] - region_bound_[2],
                          region_bound_[5] - region_bound_[4]};
  int lower_bound[3];
  lower_bound[0] = int(particle->position[0] - region_bound_[0]);
  lower_bound[0] = std::min(int(region_bound_[1] - region_bound_[0]) - 2, lower_bound[0]);
  lower_bound[1] = int(particle->position[1] - region_bound_[2]);
  lower_bound[1] = std::min(int(region_bound_[3] - region_bound_[2]) - 2, lower_bound[1]);
  lower_bound[2] = int(particle->position[2] - region_bound_[4]);
  lower_bound[2] = std::min(int(region_bound_[5] - region_bound_[4]) - 2, lower_bound[2]);
  assert(lower_bound[0] >= 0 && lower_bound[1] >= 0 && lower_bound[2] >= 0);
  float ratio_xyz[3] = {particle->position[0] - region_bound_[0] - float(lower_bound[0]),
                        particle->position[1] - region_bound_[2] - float(lower_bound[1]),
                        particle->position[2] - region_bound_[4] - float(lower_bound[2])};
  float ratio_000 = ratio_xyz[0] * ratio_xyz[1] * ratio_xyz[2];
  float ratio_001 = ratio_xyz[0] * ratio_xyz[1] * (1.0 - ratio_xyz[2]);
  float ratio_010 = ratio_xyz[0] * (1.0 - ratio_xyz[1]) * ratio_xyz[2];
  float ratio_011 = ratio_xyz[0] * (1.0 - ratio_xyz[1]) * (1.0 - ratio_xyz[2]);
  float ratio_100 = (1.0 - ratio_xyz[0]) * ratio_xyz[1] * ratio_xyz[2];
  float ratio_101 = (1.0 - ratio_xyz[0]) * ratio_xyz[1] * (1.0 - ratio_xyz[2]);
  float ratio_110 = (1.0 - ratio_xyz[0]) * (1.0 - ratio_xyz[1]) * ratio_xyz[2];
  float ratio_111 = (1.0 - ratio_xyz[0]) * (1.0 - ratio_xyz[1]) * (1.0 - ratio_xyz[2]);
  int index_000 = lower_bound[0] + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_001 = lower_bound[0] + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  int index_010 = lower_bound[0] + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_011 = lower_bound[0] + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  int index_100 = (lower_bound[0] + 1) + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_101 = (lower_bound[0] + 1) + region_size[0] * lower_bound[1]
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  int index_110 = (lower_bound[0] + 1) + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * lower_bound[2];
  int index_111 = (lower_bound[0] + 1) + region_size[0] * (lower_bound[1] + 1)
                + region_size[0] * region_size[1] * (lower_bound[2] + 1);
  particle->scalars[0] = flow_field_[3][index_000] * ratio_111 + flow_field_[3][index_001] * ratio_110
                       + flow_field_[3][index_010] * ratio_101 + flow_field_[3][index_011] * ratio_100
                       + flow_field_[3][index_100] * ratio_011 + flow_field_[3][index_101] * ratio_010
                       + flow_field_[3][index_110] * ratio_001 + flow_field_[3][index_111] * ratio_000;
  particle->scalars[0] *= 10.0;
  particle->scalars[1] = flow_field_[4][index_000] * ratio_111 + flow_field_[4][index_001] * ratio_110
                       + flow_field_[4][index_010] * ratio_101 + flow_field_[4][index_011] * ratio_100
                       + flow_field_[4][index_100] * ratio_011 + flow_field_[4][index_101] * ratio_010
                       + flow_field_[4][index_110] * ratio_001 + flow_field_[4][index_111] * ratio_000;
  particle->scalars[2] = flow_field_[5][index_000] * ratio_111 + flow_field_[5][index_001] * ratio_110
                       + flow_field_[5][index_010] * ratio_101 + flow_field_[5][index_011] * ratio_100
                       + flow_field_[5][index_100] * ratio_011 + flow_field_[5][index_101] * ratio_010
                       + flow_field_[5][index_110] * ratio_001 + flow_field_[5][index_111] * ratio_000;
  if (!(lower_bound[0] < int(region_bound_[1] - region_bound_[0]) - 1
      && lower_bound[1] < int(region_bound_[3] - region_bound_[2]) - 1
      && lower_bound[2] < int(region_bound_[5] - region_bound_[4]) - 1))
  {
//    std::cout << particle->scalars[0] << std::endl;
  }
  assert(lower_bound[0] < int(region_bound_[1] - region_bound_[0]) - 1
      && lower_bound[1] < int(region_bound_[3] - region_bound_[2]) - 1
      && lower_bound[2] < int(region_bound_[5] - region_bound_[4]) - 1);
}

bool Simulator::isParticleInside(const Particle& particle) const
{
  for (int i = 0; i < 3; ++i)
    if (particle.position[i] < region_bound_[2 * i]
     || particle.position[i] >= region_bound_[2 * i + 1] + 1.0)
      return false;
  return true;
}

void Simulator::communicateWithNeighbors()
{
  int this_rank_region = regionIndexToRank(region_index_);
  int this_rank_mpi;
  MPI_Comm_rank(MPI_COMM_WORLD, &this_rank_mpi);
  assert(this_rank_region == this_rank_mpi);
//  std::cout << "Rank: " << this_rank_region << " start communicate." << std::endl;
  // catagorize the particles
  std::map<int, std::vector<unsigned int> > map_rank_particles;
  for (unsigned int i = 0; i < leaving_particles_next_.size(); ++i)
  {
    Particle particle = leaving_particles_next_[i];
    int next_region_index[3] = {region_index_[0],
                                region_index_[1],
                                region_index_[2]};
    for (int j = 0; j < 3; ++j)
    {
      if (particle.position[j] < region_bound_[2 * j])
        --next_region_index[j];
      else if (particle.position[j] >= region_bound_[2 * j + 1])
        ++next_region_index[j];
    }
    if (next_region_index[0] >= 0 && next_region_index[0] < region_count_[0]
     && next_region_index[1] >= 0 && next_region_index[1] < region_count_[1]
     && next_region_index[2] >= 0 && next_region_index[2] < region_count_[2])
    {
      int next_rank = regionIndexToRank(next_region_index);
      map_rank_particles[next_rank].push_back(i);
//      std::cout << "    Rank: " << this_rank_region << " Next Rank: " << next_rank << std::endl;
    }
  }
  // send
  std::vector<int> neighbor_ranks = getNeighborRanks();
  for (unsigned int i = 0; i < neighbor_ranks.size(); ++i)
  {
    if (map_rank_particles.count(neighbor_ranks[i]) == 0)
    { // no particle is leaving to that region
      int zero = 0;
      MPI_Send(&zero, 1, MPI_INT, neighbor_ranks[i], TAG_TIMESTEP, MPI_COMM_WORLD);
    } else
    { // there is some particle that is going there
      std::vector<unsigned int> ps = map_rank_particles[neighbor_ranks[i]];
      int particle_count = ps.size();
      MPI_Send(&particle_count, 1, MPI_INT, neighbor_ranks[i], TAG_TIMESTEP, MPI_COMM_WORLD);
      float* data = new float [particle_count * 6 * 2];
      int* id = new int [particle_count * 2];
      for (int j = 0; j < particle_count; ++j)
      { // current
        Particle p1 = leaving_particles_current_[ps[j]];
        fillParticleScalars(&p1);
        data[j * 6 * 2 + 0] = p1.position[0];
        data[j * 6 * 2 + 1] = p1.position[1];
        data[j * 6 * 2 + 2] = p1.position[2];
        data[j * 6 * 2 + 3] = p1.scalars[0];
        data[j * 6 * 2 + 4] = p1.scalars[1];
        data[j * 6 * 2 + 5] = p1.scalars[2];
        id[j * 2 + 0] = p1.tube_id;
        // next
        data[j * 6 * 2 + 6] = leaving_particles_next_[ps[j]].position[0];
        data[j * 6 * 2 + 7] = leaving_particles_next_[ps[j]].position[1];
        data[j * 6 * 2 + 8] = leaving_particles_next_[ps[j]].position[2];
        data[j * 6 * 2 + 9] = leaving_particles_next_[ps[j]].scalars[0];
        data[j * 6 * 2 + 10] = leaving_particles_next_[ps[j]].scalars[1];
        data[j * 6 * 2 + 11] = leaving_particles_next_[ps[j]].scalars[2];
        id[j * 2 + 1] = leaving_particles_next_[ps[j]].tube_id;
      }
//      std::cout << "    Rank: " << this_rank_region
//                << " sending " << particle_count
//                << " particles to neighbor " << neighbor_ranks[i] << std::endl;
      MPI_Send(data, particle_count * 6 * 2, MPI_FLOAT, neighbor_ranks[i],
               TAG_TIMESTEP_DATA, MPI_COMM_WORLD);
      MPI_Send(id, particle_count * 2, MPI_INT, neighbor_ranks[i],
               TAG_TIMESTEP_DATA, MPI_COMM_WORLD);
      delete [] data;
      delete [] id;
    }
  }
  // receive
  inc_particles_current_.clear();
  inc_particles_next_.clear();
  for (unsigned int i = 0; i < neighbor_ranks.size(); ++i)
  {
    MPI_Status status;
    int particle_count;
    MPI_Recv(&particle_count, 1, MPI_INT, neighbor_ranks[i], TAG_TIMESTEP, MPI_COMM_WORLD, &status);
    if (particle_count == 0)
      continue;
    float* data = new float [particle_count * 6 * 2];
    int* id = new int [particle_count * 2];
    std::cout << "    Rank: " << this_rank_region
              << " receiving " << particle_count
              << " particles from neighbor " << neighbor_ranks[i] << std::endl;
    MPI_Recv(data, particle_count * 6 * 2, MPI_FLOAT, neighbor_ranks[i],
             TAG_TIMESTEP_DATA, MPI_COMM_WORLD, &status);
    MPI_Recv(id, particle_count * 2, MPI_INT, neighbor_ranks[i],
             TAG_TIMESTEP_DATA, MPI_COMM_WORLD, &status);
    for (int j = 0; j < particle_count; ++j)
    {
      Particle p1, p2;
      p1.position[0] = data[j * 6 * 2 + 0];
      p1.position[1] = data[j * 6 * 2 + 1];
      p1.position[2] = data[j * 6 * 2 + 2];
      p1.scalars[0] = data[j * 6 * 2 + 3];
      p1.scalars[1] = data[j * 6 * 2 + 4];
      p1.scalars[2] = data[j * 6 * 2 + 5];
      p1.tube_id = id[j * 2 + 0];

      p2.position[0] = data[j * 6 * 2 + 6];
      p2.position[1] = data[j * 6 * 2 + 7];
      p2.position[2] = data[j * 6 * 2 + 8];
      p2.scalars[0] = data[j * 6 * 2 + 9];
      p2.scalars[1] = data[j * 6 * 2 + 10];
      p2.scalars[2] = data[j * 6 * 2 + 11];
      p2.tube_id = id[j * 2 + 1];

      if (!isParticleInside(p2))
        continue;
      assert(isParticleInside(p2));
      inc_particles_current_.push_back(p1);
      fillParticleScalars(&p2);
      inc_particles_next_.push_back(p2);
    }
//    std::cout << "    Rank: " << this_rank_region
//              << " Particles_: ";
//    for (unsigned int j = 0; j < particles_.size(); ++j)
//    {
//      std::cout << particles_[j].position[0] << ", " << particles_[j].position[1] << ", " << particles_[j].position[2] << ";; ";
//    }
//    std::cout << std::endl;
    delete [] data;
    delete [] id;
  }
}

void Simulator::writeToFile()
{
  std::vector<Particle> ps1, ps2;
  ps1 = particles_current_;
  ps2 = particles_next_;
  assert(ps1.size() == ps2.size());
  ps1.insert(ps1.end(), inc_particles_current_.begin(), inc_particles_current_.end());
  ps2.insert(ps2.end(), inc_particles_next_.begin(), inc_particles_next_.end());
  assert(ps1.size() == ps2.size());

//  write(ps1, ps2);
  sendtoinsitu(ps1, ps2);

  particles_current_ = ps2;
}

std::vector<int> Simulator::getNeighborRanks() const
{
  std::vector<int> ranks;
  for (int i = -1; i <= 1; ++i)
    for (int j = -1; j <= 1; ++j)
      for (int k = -1; k <= 1; ++k)
      {
        if (i == 0 && j == 0 && k == 0)
          continue;
        int index3[3] = {region_index_[0] + i,
                         region_index_[1] + j,
                         region_index_[2] + k};
        int rank = regionIndexToRank(index3);
        if (rank >= 0 && rank < region_count_[0] * region_count_[1] * region_count_[2])
          ranks.push_back(rank);
      }
  return ranks;
}

bool Simulator::read(int timestep)
{
  int gsizes[3], psizes[3], lsizes[3], dims[3], periods[3], start_indices[3], local_array_size;
  MPI_Datatype filetype;
  MPI_File fh;
  MPI_Status status;

  gsizes[0] = global_size_[0];
  gsizes[1] = global_size_[1];
  gsizes[2] = global_size_[2];

  psizes[0] = region_count_[0]; // no. of processes in vertical dimension
  psizes[1] = region_count_[1]; // no. of processes in horizontal dimension
  psizes[2] = region_count_[2];

  lsizes[0] = gsizes[0] / psizes[0];
  lsizes[1] = gsizes[1] / psizes[1];
  lsizes[2] = gsizes[2] / psizes[2];
  local_array_size = lsizes[0] * lsizes[1] * lsizes[2];

  start_indices[0] = region_index_[0] * lsizes[0];
  start_indices[1] = region_index_[1] * lsizes[1];
  start_indices[2] = region_index_[2] * lsizes[2];

  MPI_Type_create_subarray(3, gsizes, lsizes, start_indices, MPI_ORDER_FORTRAN, MPI_FLOAT, &filetype);
  MPI_Type_commit(&filetype);

  char timestep_string[10];
  sprintf(timestep_string, "%4d", timestep);

  for (int i = 0; i < 6; ++i)
  {
    std::string filename = root_ + in_attributes_[i] + timestep_string + ".dat";
//    std::cout << filename << i << std::endl;
    char* cfile = &filename[0];
    if (flow_field_[i])
      delete [] flow_field_[i];
    flow_field_[i] = new float [local_array_size];

    MPI_File_open(MPI_COMM_WORLD, cfile, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, 0, MPI_FLOAT, filetype, "native", MPI_INFO_NULL);

//    std::cout << "begin" << std::endl;
    MPI_File_read_all(fh, flow_field_[i], local_array_size, MPI_FLOAT, &status);
//    std::cout << "end" << std::endl;

    MPI_File_close(&fh);
  }


/*
  char timestep_string[10];
  sprintf(timestep_string, "%4d", timestep);
  int region_rank = regionIndexToRank(region_index_);
  char region_rank_string[20];
  sprintf(region_rank_string, "region_%03d/", region_rank);
  for (int i = 0; i < 6; ++i)
  {
    std::string filename = root_ + region_rank_string + in_attributes_[i]
                         + "/" + in_attributes_[i] + timestep_string + ".dat";
    std::ifstream fin;
    fin.open(filename.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!fin.good())
      return false;
    if (flow_field_[i])
      delete [] flow_field_[i];
    int region_size[3] = {int(region_bound_[1] - region_bound_[0] + 0.5),
                          int(region_bound_[3] - region_bound_[2] + 0.5),
                          int(region_bound_[5] - region_bound_[4] + 0.5)};
    flow_field_[i] = new float [region_size[0] * region_size[1] * region_size[2]];
    fin.read(reinterpret_cast<char *>(flow_field_[i]),
             region_size[0] * region_size[1] * region_size[2] * sizeof(float));
  }
*/
  return true;
}

bool Simulator::write(const std::vector<Particle>& particles1, const std::vector<Particle>& particles2) const
{
  float* data1[6]; // x, y, z, density, entropy, pressure
  float* data2[6];
  int* id1 = new int [particles1.size()];
  int* id2 = new int [particles2.size()];
  for (int i = 0; i < 6; ++i)
  {
    data1[i] = new float [particles1.size()];
    data2[i] = new float [particles2.size()];
  }
  for (unsigned int i = 0; i < particles1.size(); ++i)
  {
    // p1
    Particle p1 = particles1[i];
    data1[0][i] = p1.position[0];
    data1[1][i] = p1.position[1];
    data1[2][i] = p1.position[2];
    data1[3][i] = p1.scalars[0];
    data1[4][i] = p1.scalars[1];
    data1[5][i] = p1.scalars[2];
    id1[i] = p1.tube_id;
    // p2
    Particle p2 = particles2[i];
    data2[0][i] = p2.position[0];
    data2[1][i] = p2.position[1];
    data2[2][i] = p2.position[2];
    data2[3][i] = p2.scalars[0];
    data2[4][i] = p2.scalars[1];
    data2[5][i] = p2.scalars[2];
    id2[i] = p2.tube_id;
  }
  char timestep_string[10];
  sprintf(timestep_string, "%05d", out_timestep_);
  char proc_string[10];
  sprintf(proc_string, "proc%02d/", regionIndexToRank(region_index_));
  for (int i = 0; i < 6; ++i)
  {
    std::string path = out_root_ + proc_string + out_attributes_[i] + "/"; 
    mkpath(path.c_str(), 0777);
    mkpath(path.c_str(), 0777);
    mkpath(path.c_str(), 0777);
    std::string filename = path + "tubes_"+ timestep_string + "_" + out_attributes_[i] + ".bin";
//    std::cout << "Write File Name: " << filename << std::endl;
    std::ofstream fout(filename.c_str(), std::ofstream::out | std::ofstream::binary);
    // write particle count
    int particle_count = particles1.size();
    fout.write(reinterpret_cast<char *>(&particle_count), sizeof(int));
    fout.write(reinterpret_cast<char *>(data1[i]), particle_count * sizeof(float));
    fout.write(reinterpret_cast<char *>(data2[i]), particle_count * sizeof(float));
    fout.close();
  }
  // id
  std::string path = out_root_ + proc_string + "id/";
  mkpath(path.c_str(), 0777);
  mkpath(path.c_str(), 0777);
  mkpath(path.c_str(), 0777);
  std::string filename = path + "tubes_" + timestep_string + "_id.bin";
  std::ofstream fout(filename.c_str(), std::ofstream::out | std::ofstream::binary);
  int particle_count = particles1.size();
  fout.write(reinterpret_cast<char *>(&particle_count), sizeof(int));
  fout.write(reinterpret_cast<char *>(id1), particle_count * sizeof(int));
  fout.write(reinterpret_cast<char *>(id2), particle_count * sizeof(int));
  fout.close();

  for (int i = 0; i < 6; ++i)
  {
    delete [] data1[i];
    delete [] data2[i];
  }
  delete [] id1;
  delete [] id2;
  return true;
}

bool Simulator::sendtoinsitu(const std::vector<Particle>& particles1, const std::vector<Particle>& particles2)
{
  std::vector<tube::Particle> p1 = translatetotubeparticle(particles1);
  std::vector<tube::Particle> p2 = translatetotubeparticle(particles2);

  static bool first_time = true;
  if (first_time)
  {
    first_time = false;
    coretube_.SetCameras(config_reader_.GetCameras());
    coretube_.SetLightPosition(config_reader_.GetLightPosition());
    double extent[6];
    for (int i = 0; i < 6; ++i)
      extent[i] = region_bound_[i];
    coretube_.SetExtent(extent);
  }

  clock_t start, end, tick;
  start = clock();
  coretube_.GenerateTubes(p1, p2);
  end = clock();
  tick = end - start;
  int milli = double(tick) / CLOCKS_PER_SEC * 1000.0;
  times_.push_back(milli);

  return true;
}

std::vector<tube::Particle> Simulator::translatetotubeparticle(const std::vector<Particle>& particles) const
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
