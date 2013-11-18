#ifndef __Particle_h__
#define __Particle_h__

#include <vector>
#include <cassert>

#include "Vector.h"
#include "VectorField.h"

template <class T = float>
class Particle
{
private:
	const static int vDim = 3;

public:
	Particle();

	// general [] getter:
	// when idx < vDim, it returns the particle position,
	// when idx >= vDim, it returns the scalar values
	T operator[](unsigned int idx) const;
	T& operator[](unsigned int idx);

	// other convenient accessors
	T x() const { return Coord[0]; }
	T& x() { return Coord[0]; }
	T y() const { return Coord[1]; }
	T& y() { return Coord[1]; }
	T z() const { return Coord[2]; }
	T& z() { return Coord[2]; }
	const Vector<vDim, T>& coord() const { return Coord; }
	Vector<vDim, T>& coord() { return Coord; }
	const std::vector<T>& scalars() const { return Scalars; }
	unsigned int numScalars() const { return Scalars.size(); }
	std::vector<T>& scalars() { return Scalars; }
	T scalar(unsigned idx) const;
	T& scalar(unsigned idx);
	unsigned int id() const { return Id; }
	unsigned int& id() { return Id; }

protected:

private:
	unsigned int Id;
	Vector<vDim, T> Coord;
	std::vector<T> Scalars;
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
Particle<T>::Particle() : Coord(vDim)
{
	Coord[0] = Coord[1] = Coord[2] = 0.0;
}

template <class T>
T Particle<T>::operator[](unsigned int idx) const
{
	if (idx < vDim)
		return Coord[idx];
	return this->scalar(idx - vDim);
}

template <class T>
T& Particle<T>::operator[](unsigned int idx)
{
	if (idx < vDim)
		return Coord[idx];
	return this->scalar(idx - vDim);
}

template <class T>
T Particle<T>::scalar(unsigned int idx) const
{
	if (idx >= Scalars.size())
		return 0.0;
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