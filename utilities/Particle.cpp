#include "Particle.h"

Particle::Particle() : coord(3)
{
	Coord[0] = Coord[1] = Coord[2] = 0.0;
}

template <class T>
T operator[](unsigned int idx) const
{
	if (idx < 3)
		return Coord[idx];
	return Scalars[idx - 3];
}

template <class T>
T& operator[](unsigned int idx)
{
	if (idx < 3)
		return Coord[idx];
	return Scalars[idx - 3];
}