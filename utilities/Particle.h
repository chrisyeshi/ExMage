#ifndef __Particle_h__
#define __Particle_h__

template <class T = float>
class Particle
{
public:
	Particle();

	// general [] getter:
	// when idx < 3, it returns the particle position,
	// when idx >= 3, it returns the scalar values
	T operator[](unsigned int idx) const;
	T& operator[](unsigned int idx);

	// other convenient accessors
	T x() const { return Coord[0]; }
	T& x() { return Coord[0]; }
	T y() const { return Coord[1]; }
	T& y() { return Coord[1]; }
	T z() const { return Coord[2]; }
	T& z() { return Coord[2]; }
	const std::vector<T>& coord() const { return Coord; }
	std::vector<T>& coord() { return Coord; }
	const std::vector<T>& scalars() const { return Scalars; }
	std::vector<T>& scalars() { return Scalars; }
	T scalar(unsigned idx) const { return Scalars[idx]; }
	T& scalar(unsigned idx) { return Scalars[idx]; }

protected:
	unsigned int Id;
	std::vector<T> Coord;
	std::vector<T> Scalars;

private:
};

#endif //__Particle_h__