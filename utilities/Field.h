#ifndef __Field_h__
#define __Field_h__

// It's a wrapper class around the real vector field buffer.
// This class does not manage the memory but only using it.

#include <vector>
#include <cassert>

#include "Vector.h"

template <class T = float>
class Field
{
public:
	Field();
	Field(const T* const buffer, int x, int y, int z);
	Field(const T* const buffer, const std::vector<int>& dimension);
	Field(const T* const buffer, int* dimension);

	void setBuffer(const T* const buffer);
	void setDimension(int x, int y, int z);
	void setDimension(const std::vector<int>& dimension);
	void setDimension(int* dimension);
	void set(const T* const buffer, int x, int y, int z);
	void set(const T* const buffer, const std::vector<int>& dimension);
	void set(const T* const buffer, int* dimension);

	T interpolate(const Vector<>& loc) const;

protected:

private:
	const T* field;
	std::vector<int> dim;
};

template <class T>
Field<T>::Field() : field(NULL)
{
	dim.resize(3);
	dim[0] = dim[1] = dim[2] = 0;
}

template <class T>
Field<T>::Field(const T* const buffer, int x, int y, int z) : field(buffer)
{
	dim.resize(3);
	dim[0] = x; dim[1] = y; dim[2] = z;
}

template <class T>
Field<T>::Field(const T* const buffer, const std::vector<int>& dimension) : field(buffer)
{
	assert(dimension.size() == 3);
	dim = dimension;
}

template <class T>
Field<T>::Field(const T* const buffer, int* dimension) : field(buffer)
{
	dim.resize(3);
	dim[0] = dimension[0]; dim[1] = dimension[1]; dim[2] = dimension[2];
}

template <class T>
void Field<T>::setBuffer(const T* const buffer)
{
	field = buffer;
}

template <class T>
void Field<T>::setDimension(int x, int y, int z)
{
	dim[0] = x; dim[1] = y; dim[2] = z;
}

template <class T>
void Field<T>::setDimension(const std::vector<int>& dimension)
{
	this->setDimension(dimension[0], dimension[1], dimension[2]);
}

template <class T>
void Field<T>::setDimension(int* dimension)
{
	this->setDimension(dimension[0], dimension[1], dimension[2]);
}

template <class T>
void Field<T>::set(const T* const buffer, int x, int y, int z)
{
	this->setBuffer(buffer);
	this->setDimension(x, y, z);
}

template <class T>
void Field<T>::set(const T* const buffer, const std::vector<int>& dimension)
{
	this->set(buffer, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void Field<T>::set(const T* const buffer, int* dimension)
{
	this->set(buffer, dimension[0], dimension[1], dimension[2]);
}

template <class T>
T Field<T>::interpolate(const Vector<>& loc) const
{
	int lower_bound[3] = {int(loc[0]), int(loc[1]), int(loc[2])};
	float ratio_xyz[3] = {loc[0] - float(lower_bound[0]),
						  loc[1] - float(lower_bound[1]),
						  loc[2] - float(lower_bound[2])};
	float ratio_000 = ratio_xyz[0] * ratio_xyz[1] * ratio_xyz[2];
	float ratio_001 = ratio_xyz[0] * ratio_xyz[1] * (1.0 - ratio_xyz[2]);
	float ratio_010 = ratio_xyz[0] * (1.0 - ratio_xyz[1]) * ratio_xyz[2];
	float ratio_011 = ratio_xyz[0] * (1.0 - ratio_xyz[1]) * (1.0 - ratio_xyz[2]);
	float ratio_100 = (1.0 - ratio_xyz[0]) * ratio_xyz[1] * ratio_xyz[2];
	float ratio_101 = (1.0 - ratio_xyz[0]) * ratio_xyz[1] * (1.0 - ratio_xyz[2]);
	float ratio_110 = (1.0 - ratio_xyz[0]) * (1.0 - ratio_xyz[1]) * ratio_xyz[2];
	float ratio_111 = (1.0 - ratio_xyz[0]) * (1.0 - ratio_xyz[1]) * (1.0 - ratio_xyz[2]);
	int index_000 = lower_bound[0] + dim[0] * lower_bound[1] + dim[0] * dim[1] * lower_bound[2];
	int index_001 = lower_bound[0] + dim[0] * lower_bound[1] + dim[0] * dim[1] * (lower_bound[2] + 1);
	int index_010 = lower_bound[0] + dim[0] * (lower_bound[1] + 1) + dim[0] * dim[1] * lower_bound[2];
	int index_011 = lower_bound[0] + dim[0] * (lower_bound[1] + 1) + dim[0] * dim[1] * (lower_bound[2] + 1);
	int index_100 = (lower_bound[0] + 1) + dim[0] * lower_bound[1] + dim[0] * dim[1] * lower_bound[2];
	int index_101 = (lower_bound[0] + 1) + dim[0] * lower_bound[1] + dim[0] * dim[1] * (lower_bound[2] + 1);
	int index_110 = (lower_bound[0] + 1) + dim[0] * (lower_bound[1] + 1) + dim[0] * dim[1] * lower_bound[2];
	int index_111 = (lower_bound[0] + 1) + dim[0] * (lower_bound[1] + 1) + dim[0] * dim[1] * (lower_bound[2] + 1);
	return field[index_000] * ratio_111 + field[index_001] * ratio_110
	     + field[index_010] * ratio_101 + field[index_011] * ratio_100
	     + field[index_100] * ratio_011 + field[index_101] * ratio_010
	     + field[index_110] * ratio_001 + field[index_111] * ratio_000;
}

#endif //__Field_h__