#ifndef __Simulator_h_
#define __Simulator_h_

#include <cstdlib>
#include <string>
#include <vector>

#include "Particle.h"
#include "CoreTube.h"
#include "ConfigReader.h"

class Particle
{
public:
  Particle() {position.resize(3); scalars.resize(3);}
  std::vector<float> position;
  std::vector<float> scalars;
  int tube_id;
};

class Simulator
{
public:
  Simulator();
  ~Simulator();

  void trace(std::vector<float*> fields);
  void output();
 
protected:
  std::vector<Particle> particles_current_;
  std::vector<Particle> particles_next_;
  int current_timestep_;
  int out_timestep_;
  std::vector<float*> flow_field_;
  std::vector<Particle> leaving_particles_current_;
  std::vector<Particle> leaving_particles_next_;
  std::vector<Particle> inc_particles_current_;
  std::vector<Particle> inc_particles_next_;
  CoreTube coretube_;
  std::vector<int> times_;

  enum READ_ERROR {READ_SUCCESS, RAW_FAIL, HDF5_FAIL};

  void initializeParticles(int particle_count);
  Particle findBoundaryParticle(const Particle& curr, const Particle& next) const;
  void traceParticles();
  Particle traceParticle(const Particle& particle) const;
  void getParticleVelocity(const Particle& particle, float velocity3[3]) const;
  void fillParticleScalars(Particle* particle) const;
  bool isParticleInside(const Particle& particle) const;
  void communicateWithNeighbors();
  void writeToFile();
  std::vector<int> getNeighborRanks() const;
  void printReadError(READ_ERROR read_error) const;
  READ_ERROR read(int timestep);
  bool write(const std::vector<Particle>& particles1, const std::vector<Particle>& particles2) const;
  bool sendtoinsitu(const std::vector<Particle>& particles1, const std::vector<Particle>& particles2);
  std::vector<tube::Particle> translatetotubeparticle(const std::vector<Particle>& particles) const;
  ConfigReader& config() const {return ConfigReader::getInstance();}
  std::vector<int> global_size() const;
  std::vector<int> region_count() const;
  std::vector<int> region_index() const;
  int total_region_count() const;
  int global_index() const;
  std::string global_index_string() const;
  std::vector<float> region_bound() const;
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
