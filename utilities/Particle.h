#ifndef __Particle_h__
#define __Particle_h__

#include <vector>

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
	T scalar(unsigned idx) const;
	T& scalar(unsigned idx);
	unsigned int id() const { return Id; }
	unsigned int& id() { return Id; }

protected:
	unsigned int Id;
	std::vector<T> Coord;
	std::vector<T> Scalars;

private:
};

////////////////////////////////////////////////////
//
//
//
//
// Implementations
//
//
//
//
////////////////////////////////////////////////////
template <class T>
Particle<T>::Particle() : Coord(3)
{
	Coord[0] = Coord[1] = Coord[2] = 0.0;
}

template <class T>
T Particle<T>::operator[](unsigned int idx) const
{
	if (idx < 3)
		return Coord[idx];
	return Scalars[idx - 3];
}

template <class T>
T& Particle<T>::operator[](unsigned int idx)
{
	if (idx < 3)
		return Coord[idx];
	return Scalars[idx - 3];
}

template <class T>
T Particle<T>::scalar(unsigned int idx) const
{
	assert(idx < Scalars.size());
	return Scalars[idx];
}

template <class T>
T& Particle<T>::scalar(unsigned int idx)
{
	if (idx >= Scalars.size())
		Scalars.resize(idx + 1);
	return Scalars[idx];
}

#endif //__Particle_h__