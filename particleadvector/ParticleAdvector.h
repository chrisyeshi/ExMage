#ifndef __ParticleAdvector_h__
#define __ParticleAdvector_h__

#include <cstdlib>
#include <string>
#include <vector>

#include "ConfigReader.h"
#include "Particle.h"
#include "VectorField.h"
#include "PtclSync.h"

class ParticleAdvector
{
public:
  ParticleAdvector();
  ~ParticleAdvector();

  void trace(std::vector<float*> fields);
  void output();
  std::vector<Particle<> > prevParticles() const;
  std::vector<Particle<> > nextParticles() const;
  std::vector<float> region_bound() const;
 
protected:
  std::vector<Particle<> > curr_;
  std::vector<Particle<> > next_;
  int timestep_;
  VectorField<> flow_;
  std::vector<Particle<> > outCurr_;
  std::vector<Particle<> > outNext_;
  std::vector<Particle<> > incCurr_;
  std::vector<Particle<> > incNext_;
  PtclSync comm_;

  void initializeParticles(int particle_count);
  void traceParticles();
  Particle<> traceParticle(const Particle<>& particle) const;
  void communicateWithNeighbors();
  ConfigReader& config() const { return ConfigReader::getInstance(); }
};

#endif
