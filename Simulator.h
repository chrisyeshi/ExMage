#ifndef __Simulator_h_
#define __Simulator_h_

#include <string>
#include <vector>

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

  void simulate(int particle_count);

  void set_region_count(int region_count[3]);
  void set_region_index(int region_index[3]);
  void set_region_bound(float region_bound[6]);
  void set_root(const std::string& root);
  void set_out_root(const std::string& out_root);
  void set_timestep_range(int timestep_range[2]);
  void set_velocity(float velocity);

protected:
  int region_count_[3];
  int region_index_[3];
  float region_bound_[6]; // xmin, xmax, ymin, ymax, zmin, zmax
  std::string root_;
  std::string out_root_;
  int timestep_range_[2];
  float velocity_;
  std::vector<Particle> particles_current_;
  std::vector<Particle> particles_next_;
  int current_timestep_;
  int out_timestep_;
  std::vector<std::string> in_attributes_;
  std::vector<std::string> out_attributes_;
  float* flow_field_[6];
  std::vector<Particle> leaving_particles_current_;
  std::vector<Particle> leaving_particles_next_;
  std::vector<Particle> inc_particles_current_;
  std::vector<Particle> inc_particles_next_;

  void initializeParticles(int particle_count);
  int regionIndexToRank(const int region_index[3]) const;
  void rankToRegionIndex(int rank, int region_index[3]) const;
  Particle findBoundaryParticle(const Particle& curr, const Particle& next) const;
  void traceParticles();
  Particle traceParticle(const Particle& particle) const;
  void getParticleVelocity(const Particle& particle, float velocity3[3]) const;
  void fillParticleScalars(Particle* particle) const;
  bool isParticleInside(const Particle& particle) const;
  void communicateWithNeighbors();
  void writeToFile();
  std::vector<int> getNeighborRanks() const;
  bool read(int timestep);
  bool write(const std::vector<Particle>& particles1, const std::vector<Particle>& particles2) const;
};

#endif
