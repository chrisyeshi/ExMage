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
#include "mkpath.h"
#include "VectorField.h"
#include "DomainInfo.h"

////////////////////////////////////////////////////////////////////////////////
//
//
//
// Public
//
//
//
////////////////////////////////////////////////////////////////////////////////

ParticleAdvector::ParticleAdvector()
{
}

ParticleAdvector::~ParticleAdvector()
{
}

void ParticleAdvector::trace(std::vector<float*> fields)
{
    std::vector<Vector<> > bounds = DomainInfo::bounds();
    Vector<> range = DomainInfo::ranges();
    flow_.set(fields, Vector<3, int>(int(range[0] + 0.5), int(range[1] + 0.5), int(range[2] + 0.5)));
    static bool first_time = true;
    if (first_time)
    {
        int particle_count = ConfigReader::getInstance().GetRegionParticleCount();
        initializeParticles(particle_count);
        timestep_ = 0;
        first_time = false;
    } else
    {
        curr_ = next_;
        curr_.insert(curr_.end(), incNext_.begin(), incNext_.end());
        next_.clear();
    }
    std::cout << "Timestep: " << timestep_++ << std::endl;
    traceParticles();
    communicateWithNeighbors();
}

std::vector<Particle<> > ParticleAdvector::prevParticles() const
{
    std::vector<Particle<> > p = curr_;
    p.insert(p.end(), outCurr_.begin(), outCurr_.end());
    p.insert(p.end(), incCurr_.begin(), incCurr_.end());
    return p;
}

std::vector<Particle<> > ParticleAdvector::nextParticles() const
{
    std::vector<Particle<> > p = next_;
    p.insert(p.end(), outNext_.begin(), outNext_.end());
    p.insert(p.end(), incNext_.begin(), incNext_.end());
    return p;
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
    curr_.resize(particle_count);
    std::vector<Vector<> > bounds = DomainInfo::bounds();
    Vector<> range = bounds[1] - bounds[0];
    for (int j = 0; j < particle_count; ++j)
    {
        for (int i = 0; i < 3; ++i)
        {
            curr_[j].coord()[i] = float(rand() % int(range[i] * 1000)) / 1000.0 + bounds[0][i];
        }
        std::vector<float> scalars = flow_.getScalars(curr_[j].coord() - bounds[0]);
        curr_[j].scalars() = scalars;
        curr_[j].scalar(0) *= 10.0;
        // id
        curr_[j].id() = j;
    }
}

void ParticleAdvector::traceParticles()
{
  outCurr_.clear();
  outNext_.clear();
  std::vector<Particle<> > particles_current;
  std::vector<Particle<> > particles_next;
  for (unsigned int i = 0; i < curr_.size(); ++i)
  {
    Particle<> next_particle = traceParticle(curr_[i]);
    if (DomainInfo::inBounds(next_particle.coord()))
    {
      particles_current.push_back(curr_[i]);
      next_particle.scalars() = flow_.getScalars(next_particle.coord() - DomainInfo::bounds()[0]);
      next_particle.scalar(0) *= 10.0;
      particles_next.push_back(next_particle);
    } else
    {
      Particle<> current_particle = curr_[i];
      outCurr_.push_back(current_particle);
      outNext_.push_back(next_particle);
    }
  }
  curr_ = particles_current;
  next_ = particles_next;
}

Particle<> ParticleAdvector::traceParticle(const Particle<>& particle) const
{
  Vector<> velocity3 = flow_.getVelocity(particle.coord() - DomainInfo::bounds()[0]);
  float multiplier = config().GetVelocity();
  Particle<> ret;
  ret.coord() = particle.coord() + velocity3 * multiplier;
  ret.id() = particle.id();
  return ret;
}

void ParticleAdvector::communicateWithNeighbors()
{
    comm_.scatter(outCurr_, outNext_, flow_);
    outNext_ = comm_.getNextOut();
    incCurr_ = comm_.getCurrInc();
    incNext_ = comm_.getNextInc();
    assert(incCurr_.size() == incNext_.size());
}