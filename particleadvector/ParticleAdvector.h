#ifndef __ParticleAdvector_h__
#define __ParticleAdvector_h__

#include <cstdlib>
#include <string>
#include <vector>

#include "ConfigReader.h"
#include "Particle.h"
#include "VectorField.h"

class ParticleAdvector
{
public:
  ParticleAdvector();
  ~ParticleAdvector();

  enum FLAT_INDEX {X_MINUS, X_PLUS, Y_MINUS, Y_PLUS, Z_MINUS, Z_PLUS};

  void trace(std::vector<float*> fields);
  void output();
  std::vector<Particle<> > prevParticles() const;
  std::vector<Particle<> > nextParticles() const;
  std::vector<float> region_bound() const;
 
protected:
  std::vector<Particle<> > particles_current_;
  std::vector<Particle<> > particles_next_;
  int current_timestep_;
  int out_timestep_;
  VectorField<> flow_field_;
  std::vector<Particle<> > leaving_particles_current_;
  std::vector<Particle<> > leaving_particles_next_;
  std::vector<Particle<> > inc_particles_current_;
  std::vector<Particle<> > inc_particles_next_;
//  CoreTube coretube_;
//  std::vector<int> times_;

  void initializeParticles(int particle_count);
  Particle<> findBoundaryParticle(const Particle<> & curr, const Particle<>& next) const;
  void traceParticles();
  Particle<> traceParticle(const Particle<>& particle) const;
  void getParticleVelocity(const Particle<>& particle, float velocity3[3]) const;
  void fillParticleScalars(Particle<>* particle) const;
  bool isParticleInside(const Particle<>& particle) const;
  void communicateWithNeighbors();
//  void writeToFile();
  std::vector<int> getNeighborRanks() const;
  bool write(const std::vector<Particle<> >& particles1, const std::vector<Particle<> >& particles2) const;
//  bool sendtoinsitu(const std::vector<Particle<> >& particles1, const std::vector<Particle<> >& particles2);
//  std::vector<tube::Particle<> > translatetotubeparticle(const std::vector<Particle<> >& particles) const;
  ConfigReader& config() const {return ConfigReader::getInstance();}
  std::vector<int> global_size() const;
  std::vector<int> region_count() const;
  std::vector<int> region_index() const;
  int total_region_count() const;
  int global_index() const;
  std::string global_index_string() const;
  std::vector<float> region_range() const;
  std::string read_root() const;
  std::string out_root() const;
  std::vector<int> timestep_range() const;
  int timestep_diff() const;
  float velocity() const;
  int particle_count() const;
  int nAttributes() const {return config().GetAttributeCount();}
  int nScalars() const {return nAttributes() - 3;}
};

#endif
