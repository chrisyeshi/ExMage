#include "ParticleAdvector.h"

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
#include "Frame.h"
#include "ConfigReader.h"
#include "ProcIndex.h"
#include "mkpath.h"
#include "VectorField.h"

extern "C" {
  int readData(const char* filename,
                int region_index[3], int region_count[3],
                char attributes[][50], int attribute_count,
                float* flow_field[6]);
}

#define TAG_TIMESTEP 1
#define TAG_TIMESTEP_DATA 2

////////////////////////////////////////////////////////////////////////////////
//
//
//
// Public
//
//
//
////////////////////////////////////////////////////////////////////////////////

ParticleAdvector::ParticleAdvector() : current_timestep_(0)
{
}

ParticleAdvector::~ParticleAdvector()
{
}

void ParticleAdvector::trace(std::vector<float*> fields)
{
    flow_field_.set(fields, int(region_range()[0] + 0.5),
                            int(region_range()[1] + 0.5),
                            int(region_range()[2] + 0.5));
    static bool first_time = true;
    if (first_time)
    {
        int particle_count = ConfigReader::getInstance().GetRegionParticleCount();
        initializeParticles(particle_count);
        out_timestep_ = 0;
        first_time = false;
    } else
    {
        particles_current_ = particles_next_;
        particles_next_.clear();
    }
    std::cout << "Timestep: " << out_timestep_;
    ++out_timestep_;
    std::cout << ", " << particles_current_.size() << std::endl;
    traceParticles();
    communicateWithNeighbors();
//    writeToFile();
}

std::vector<Particle<> > ParticleAdvector::prevParticles() const
{
    // std::cout << particles_current_.size() << std::endl;
    return particles_current_;
    // std::vector<Particle<> > p = particles_current_;
    // p.insert(p.end(), inc_particles_current_.begin(), inc_particles_current_.end());
    // return p;
}

std::vector<Particle<> > ParticleAdvector::nextParticles() const
{
    return particles_next_;
    // std::vector<Particle<> > p = particles_next_;
    // p.insert(p.end(), inc_particles_next_.begin(), inc_particles_next_.end());
    // std::cout << inc_particles_next_.size() << " p.size() = " << p.size() << std::endl;
    // return p;
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

void ParticleAdvector::initializeParticles(int particle_count)
{
    particles_current_.resize(particle_count);
    for (int j = 0; j < particle_count; ++j)
    {
        for (int i = 0; i < 3; ++i)
        {
            particles_current_[j].coord()[i] = float(rand() % int(region_range()[i] * 1000)) / 1000.0 + region_bound()[i * 2];
        }
        fillParticleScalars(&particles_current_[j]);
        // id
        particles_current_[j].id() = j;
    }
}

Particle<> ParticleAdvector::findBoundaryParticle(const Particle<>& curr, const Particle<>& next) const
{
//  for (int i = 0; i < 3; ++i)
//    if (particle.coord()[i] < region_bound_[2 * i]
//     || particle.coord()[i] >= region_bound_[2 * i + 1] + 1.0)
//      return false;
//  return true;
  float vector_full[3];
  for (int i = 0; i < 3; ++i)
    vector_full[i] = next.coord()[i] - curr.coord()[i];
  float ratio = 1.0;
  for (int i = 0; i < 3; ++i)
  {
    float r = 1.0;
    if (vector_full[i] < 0)
    {
      if (next.coord()[i] < region_bound()[2 * i])
        r = (region_bound()[2 * i] - curr.coord()[i]) / (next.coord()[i] - curr.coord()[i]);
    } else if (vector_full[i] > 0)
    {
      if (next.coord()[i] >= region_bound()[2 * i + 1] + 1.0)
        r = (region_bound()[2 * i + 1] + 1.0 - curr.coord()[i]) / (next.coord()[i] - curr.coord()[i]);
    }
    ratio = std::min(r, ratio);
  }
  Particle<> ret;
  for (int i = 0; i < 3; ++i)
    ret.coord()[i] = curr.coord()[i] + vector_full[i] * ratio;
  ret.id() = curr.id();
  return ret;
}

void ParticleAdvector::traceParticles()
{
  leaving_particles_current_.clear();
  leaving_particles_next_.clear();
  std::vector<Particle<> > particles_current;
  std::vector<Particle<> > particles_next;
  for (unsigned int i = 0; i < particles_current_.size(); ++i)
  {
    Particle<> next_particle = traceParticle(particles_current_[i]);
    if (isParticleInside(next_particle))
    {
      particles_current.push_back(particles_current_[i]);
      fillParticleScalars(&next_particle);
      particles_next.push_back(next_particle);
    } else
    {
      Particle<> current_particle = particles_current_[i];
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

Particle<> ParticleAdvector::traceParticle(const Particle<>& particle) const
{
  float velocity3[3];
  getParticleVelocity(particle, velocity3);
  // std::cout << "Velocity: " << velocity3[0] << ", " << velocity3[1] << ", " << velocity3[2] << std::endl;
  Particle<> ret;
  ret.coord()[0] = particle.coord()[0] + velocity3[0] * velocity();
  ret.coord()[1] = particle.coord()[1] + velocity3[1] * velocity();
  ret.coord()[2] = particle.coord()[2] + velocity3[2] * velocity();
  ret.id() = particle.id();
  return ret;
}

void ParticleAdvector::getParticleVelocity(const Particle<>& particle, float velocity3[3]) const
{
    Vector<> region_min(region_bound()[0], region_bound()[2], region_bound()[4]);
    Vector<> velocity = flow_field_.getVelocity(particle.coord() - region_min);
    for (int i = 0; i < 3; ++i)
        velocity3[i] = velocity[i];
}

void ParticleAdvector::fillParticleScalars(Particle<>* particle) const
{
    Vector<> region_min(region_bound()[0], region_bound()[2], region_bound()[4]);
    std::vector<float> scalars = flow_field_.getScalars(particle->coord() - region_min);
    for (int i = 0; i < scalars.size(); ++i)
        particle->scalar(i) = scalars[i];
    particle->scalar(0) *= 10.0;
}

bool ParticleAdvector::isParticleInside(const Particle<>& particle) const
{
    for (int i = 0; i < 3; ++i)
        if (particle.coord()[i] < region_bound()[2 * i]
         || particle.coord()[i] >= region_bound()[2 * i + 1] + 1.0)
            return false;
    return true;
}

void ParticleAdvector::communicateWithNeighbors()
{
    comm_.scatter(leaving_particles_current_, leaving_particles_next_, flow_field_);
    inc_particles_current_ = comm_.getCurrInc();
    inc_particles_next_ = comm_.getNextInc();
    // std::cout << inc_particles_current_.size() << " :::: " << inc_particles_next_.size() << std::endl;
    assert(inc_particles_current_.size() == inc_particles_next_.size());
    particles_current_.insert(particles_current_.end(), inc_particles_current_.begin(), inc_particles_current_.end());
    particles_next_.insert(particles_next_.end(), inc_particles_next_.begin(), inc_particles_next_.end());
    assert(particles_current_.size() == particles_next_.size());
/*
    // catagorize the particles
    std::map<int, std::vector<unsigned int> > map_rank_particles;
    for (unsigned int i = 0; i < leaving_particles_next_.size(); ++i)
    {
        Particle<> particle = leaving_particles_next_[i];
        std::vector<int> next_region_index = region_index();
        for (int j = 0; j < 3; ++j)
        {
            if (particle.coord()[j] < region_bound()[2 * j])
            {
                --next_region_index[j];
                break;
            }
            else if (particle.coord()[j] >= region_bound()[2 * j + 1])
            {
                ++next_region_index[j];
                break;
            }
        }
        if (next_region_index[0] >= 0 && next_region_index[0] < region_count()[0]
         && next_region_index[1] >= 0 && next_region_index[1] < region_count()[1]
         && next_region_index[2] >= 0 && next_region_index[2] < region_count()[2])
        {
            ProcIndex procIndex(next_region_index);
            map_rank_particles[procIndex.getGlobalIndex()].push_back(i);
        }
    }
  // send
  std::vector<int> neighbor_ranks = getNeighborRanks();
//  std::cout << "Rank: " << global_index() << " :: ";
//  for (unsigned int i = 0; i < neighbor_ranks.size(); ++i)
//  {
//      std::cout << neighbor_ranks[i] << ", ";
//  }
//  std::cout << std::endl;
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
      float* data = new float [particle_count * nAttributes() * 2];
      int* id = new int [particle_count * 2];
      for (int j = 0; j < particle_count; ++j)
      { // current
        Particle<> p1 = leaving_particles_current_[ps[j]];
        fillParticleScalars(&p1);
        int inc = 0;
        data[j * nAttributes() * 2 + inc++] = p1.coord()[0];
        data[j * nAttributes() * 2 + inc++] = p1.coord()[1];
        data[j * nAttributes() * 2 + inc++] = p1.coord()[2];
        for (int k = 0; k < nScalars(); ++k)
            data[j * nAttributes() * 2 + inc++] = p1.scalar(k);
        id[j * 2 + 0] = p1.id();
        // next
        data[j * nAttributes() * 2 + inc++] = leaving_particles_next_[ps[j]].coord()[0];
        data[j * nAttributes() * 2 + inc++] = leaving_particles_next_[ps[j]].coord()[1];
        data[j * nAttributes() * 2 + inc++] = leaving_particles_next_[ps[j]].coord()[2];
        for (int k = 0; k < nScalars(); ++k)
            data[j * nAttributes() * 2 + inc++] = leaving_particles_next_[ps[j]].scalar(k);
        id[j * 2 + 1] = leaving_particles_next_[ps[j]].id();
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
    ProcIndex procIndex;
    std::cout << "    Rank: " << procIndex.getGlobalIndex()
              << " receiving " << particle_count
              << " particles from neighbor " << neighbor_ranks[i] << std::endl;
    MPI_Recv(data, particle_count * 6 * 2, MPI_FLOAT, neighbor_ranks[i],
             TAG_TIMESTEP_DATA, MPI_COMM_WORLD, &status);
    MPI_Recv(id, particle_count * 2, MPI_INT, neighbor_ranks[i],
             TAG_TIMESTEP_DATA, MPI_COMM_WORLD, &status);
    for (int j = 0; j < particle_count; ++j)
    {
      Particle<> p1, p2;
      p1.coord()[0] = data[j * 6 * 2 + 0];
      p1.coord()[1] = data[j * 6 * 2 + 1];
      p1.coord()[2] = data[j * 6 * 2 + 2];
      p1.scalar(0) = data[j * 6 * 2 + 3];
      p1.scalar(1) = data[j * 6 * 2 + 4];
      p1.scalar(2) = data[j * 6 * 2 + 5];
      p1.id() = id[j * 2 + 0];

      p2.coord()[0] = data[j * 6 * 2 + 6];
      p2.coord()[1] = data[j * 6 * 2 + 7];
      p2.coord()[2] = data[j * 6 * 2 + 8];
      p2.scalar(0) = data[j * 6 * 2 + 9];
      p2.scalar(1) = data[j * 6 * 2 + 10];
      p2.scalar(2) = data[j * 6 * 2 + 11];
      p2.id() = id[j * 2 + 1];

      if (!isParticleInside(p2))
        continue;
      assert(isParticleInside(p2));
      inc_particles_current_.push_back(p1);
      fillParticleScalars(&p2);
      inc_particles_next_.push_back(p2);
    }

    delete [] data;
    delete [] id;
  }
*/
}

std::vector<int> ParticleAdvector::getNeighborRanks() const
{
    std::vector<int> ranks;
    for (int i = 0; i < 3; ++i)
    {
        std::vector<int> minus = region_index();
        minus[i] -= 1;
        if (minus[i] >= 0 && minus[i] < region_count()[i])
        {
            ProcIndex minus_index(minus);
            int minus_rank = minus_index.getGlobalIndex();
            ranks.push_back(minus_rank);
        }

        std::vector<int> plus = region_index();
        plus[i] += 1;
        if (plus[i] >= 0 && plus[i] < region_count()[i])
        {
            ProcIndex plus_index(plus);
            int plus_rank = plus_index.getGlobalIndex();
            ranks.push_back(plus_rank);
        }
    }

    return ranks;
}

bool ParticleAdvector::write(const std::vector<Particle<> >& particles1, const std::vector<Particle<> >& particles2) const
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
    Particle<> p1 = particles1[i];
    data1[0][i] = p1.coord()[0];
    data1[1][i] = p1.coord()[1];
    data1[2][i] = p1.coord()[2];
    data1[3][i] = p1.scalar(0);
    data1[4][i] = p1.scalar(1);
    data1[5][i] = p1.scalar(2);
    id1[i] = p1.id();
    // p2
    Particle<> p2 = particles2[i];
    data2[0][i] = p2.coord()[0];
    data2[1][i] = p2.coord()[1];
    data2[2][i] = p2.coord()[2];
    data2[3][i] = p2.scalar(0);
    data2[4][i] = p2.scalar(1);
    data2[5][i] = p2.scalar(2);
    id2[i] = p2.id();
  }
  char timestep_string[10];
  sprintf(timestep_string, "%05d", out_timestep_);
  char proc_string[10];
  sprintf(proc_string, "/proc%02d/", global_index());
  std::vector<std::string> out_attributes = config().GetOutputAttributes();
  for (int i = 0; i < 6; ++i)
  {
    std::string path = out_root() + proc_string + out_attributes[i] + "/"; 
    mkpath(path.c_str(), 0777);
    mkpath(path.c_str(), 0777);
    mkpath(path.c_str(), 0777);
    std::string filename = path + "tubes_"+ timestep_string + "_" + out_attributes[i] + ".bin";
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
  std::string path = out_root() + proc_string + "id/";
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

std::vector<int> ParticleAdvector::global_size() const
{
    return config().GetTotalSize();
}

std::vector<int> ParticleAdvector::region_count() const
{
    return config().GetRegionCount();
}

int ParticleAdvector::total_region_count() const
{
    return region_count()[0] * region_count()[1] * region_count()[2];
}

std::vector<int> ParticleAdvector::region_index() const
{
    ProcIndex procIndex;
    return procIndex.getRegionIndex();
}

int ParticleAdvector::global_index() const
{
    ProcIndex procIndex;
    return procIndex.getGlobalIndex();
}

std::string ParticleAdvector::global_index_string() const
{
    char gis[10];
    sprintf(gis, "%02d", global_index());
    return std::string(gis);
}

std::vector<float> ParticleAdvector::region_bound() const
{
    std::vector<float> rb(6);
    rb[0] = global_size()[0] / region_count()[0] * (region_index()[0] + 0);
    rb[1] = global_size()[0] / region_count()[0] * (region_index()[0] + 1);
    rb[2] = global_size()[1] / region_count()[1] * (region_index()[1] + 0);
    rb[3] = global_size()[1] / region_count()[1] * (region_index()[1] + 1);
    rb[4] = global_size()[2] / region_count()[2] * (region_index()[2] + 0);
    rb[5] = global_size()[2] / region_count()[2] * (region_index()[2] + 1);
    return rb;
}

std::string ParticleAdvector::read_root() const
{
    return config().GetReadRoot();
}

std::string ParticleAdvector::out_root() const
{
    return config().GetOutRoot();
}

std::vector<int> ParticleAdvector::timestep_range() const
{
    return config().GetTimeStepRange();
}

float ParticleAdvector::velocity() const
{
    return config().GetVelocity();
}

int ParticleAdvector::particle_count() const
{
    return config().GetRegionParticleCount();
}

int ParticleAdvector::timestep_diff() const
{
    return timestep_range()[1] - timestep_range()[0];
}

std::vector<float> ParticleAdvector::region_range() const
{
    std::vector<float> range(3);
    range[0] = region_bound()[1] - region_bound()[0];
    range[1] = region_bound()[3] - region_bound()[2];
    range[2] = region_bound()[5] - region_bound()[4];
    return range;
}
