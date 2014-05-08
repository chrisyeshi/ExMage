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
        int particle_count = config().get("tube.count").asNumber<int>();
        initializeParticles(particle_count);
        timestep_ = 0;
        first_time = false;
    } else
    {
        curr_ = next_;
        curr_.insert(curr_.end(), incNext_.begin(), incNext_.end());
        next_.clear();
    }
    traceParticles();
    communicateWithNeighbors();
}

std::vector<Particle<> > ParticleAdvector::prevParticles() const
{
    std::vector<Particle<> > p = curr_;
    // We don't need to append the inBorder because it is already in curr_.
    p.insert(p.end(), outBorderCurr_.begin(), outBorderCurr_.end());
    p.insert(p.end(), outCurr_.begin(), outCurr_.end());
    p.insert(p.end(), incCurr_.begin(), incCurr_.end());
    for (unsigned int i = 0; i < p.size(); ++i)
        p[i].scalar(0) *= 10.0;
    return p;
}

std::vector<Particle<> > ParticleAdvector::nextParticles() const
{
    std::vector<Particle<> > p = next_;
    // We don't need to append the inBorder because it is already in curr_.
    p.insert(p.end(), outBorderNext_.begin(), outBorderNext_.end());
    p.insert(p.end(), outNext_.begin(), outNext_.end());
    p.insert(p.end(), incNext_.begin(), incNext_.end());
    for (unsigned int i = 0; i < p.size(); ++i)
        p[i].scalar(0) *= 10.0;
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
    Vector<> range = bounds[1] - bounds[0] - 1;
    for (int j = 0; j < particle_count; ++j)
    {
        for (int i = 0; i < 3; ++i)
        {
            curr_[j].coord()[i] = float(rand() % int(range[i] * 1000)) / 1000.0 + bounds[0][i];
        }
        std::vector<float> scalars = flow_.getScalars(curr_[j].coord() - bounds[0]);
        curr_[j].scalars() = scalars;
        // curr_[j].scalar(0) *= 10.0;
        // id
        curr_[j].id() = j;
    }
}

void ParticleAdvector::traceParticles()
{
    outCurr_.clear();
    outNext_.clear();
    inBorderCurr_.clear();
    inBorderNext_.clear();
    std::vector<Particle<> > particles_current;
    std::vector<Particle<> > particles_next;
    for (unsigned int i = 0; i < curr_.size(); ++i)
    {
        Particle<> curr_particle = curr_[i];
        Particle<> next_particle = traceParticle(curr_particle);
        if (DomainInfo::inBounds(next_particle.coord()))
        { // particles that stay in this domain
            particles_current.push_back(curr_particle);
            next_particle.scalars() = flow_.getScalars(next_particle.coord() - DomainInfo::bounds()[0]);
            particles_next.push_back(next_particle);
            // border particles -- the inBorder particles are already in particles_current and particles_next
            if (DomainInfo::inBorder(curr_particle.coord()) || DomainInfo::inBorder(next_particle.coord()))
            {
                inBorderCurr_.push_back(curr_particle);
                inBorderNext_.push_back(next_particle);
            }

        } else
        { // particles that travels to other domains
            outCurr_.push_back(curr_particle);
            outNext_.push_back(next_particle);
        }
    }
    curr_ = particles_current;
    next_ = particles_next;
}

Particle<> ParticleAdvector::traceParticle(const Particle<>& particle) const
{
  Vector<> velocity3 = flow_.getVelocity(particle.coord() - DomainInfo::bounds()[0]);
  float multiplier = config().get("tube.velocity").asNumber<float>();
  Particle<> ret;
  ret.coord() = particle.coord() + velocity3 * multiplier;
  ret.id() = particle.id();
  return ret;
}

void ParticleAdvector::communicateWithNeighbors()
{
    comm_.scatter(inBorderCurr_, inBorderNext_, outCurr_, outNext_, flow_);
    outBorderCurr_ = comm_.getCurrOutBorder();
    outBorderNext_ = comm_.getNextOutBorder();
    assert(outBorderCurr_.size() == outBorderNext_.size());
    outNext_ = comm_.getNextOut();
    incCurr_ = comm_.getCurrInc();
    incNext_ = comm_.getNextInc();
    assert(incCurr_.size() == incNext_.size());
}