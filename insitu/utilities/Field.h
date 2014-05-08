#ifndef __Field_h__
#define __Field_h__

// It's a wrapper class around the real vector field buffer.
// This class does not manage the memory but only using it.
//
// Dimension in this class is tricky...
// When the Data Location is set to Vertex, dimension represents the grid dimension
// When the Data Location is set to Center, dimension represents the cell dimension
// use gridDim() and cellDim() as appropriate since the automatically calculate the values

#include <vector>
#include <cassert>
#include <limits>

#include "Vector.h"

template <class T = float>
class Field
{
public:
	enum DataLoc { Center, Vertex };

	Field(DataLoc dataLoc = Center);
	Field(const T* const buffer, const Vector<3, int>& dimension, DataLoc dataLoc = Center);

	void setBuffer(const T* const buffer) { field = buffer; }
	void setDataLoc(DataLoc dataLoc) { this->dataLoc = dataLoc; }
	void setDimension(const Vector<3, int>& dimension) { dim = dimension; }
	void set(const T* const buffer, const Vector<3, int>& dimension, DataLoc = Center);

	template <class U>
	T interpolate(const Vector<3, U>& loc) const;

protected:
	Vector<3, int> gridDim() const;
	Vector<3, int> cellDim() const;
	template <class U>
	T gridInterpolate(const Vector<3, U>& loc) const;
	template <class U>
	T cellInterpolate(const Vector<3, U>& loc) const;
	template <class U>
	T interpolate(const Vector<3, U>& loc, const Vector<3, int>& dimension) const;

private:
	const T* field;
	Vector<3, int> dim;
	DataLoc dataLoc;
};

template <class T>
Field<T>::Field(DataLoc dataLoc) : field(NULL), dataLoc(dataLoc)
{
	dim[0] = dim[1] = dim[2] = 0;
}

template <class T>
Field<T>::Field(const T* const buffer, const Vector<3, int>& dimension, DataLoc dataLoc)
  : field(buffer), dataLoc(dataLoc), dim(dimension)
{
}

template <class T>
void Field<T>::set(const T* const buffer, const Vector<3, int>& dimension, DataLoc dataLoc)
{
	this->setBuffer(buffer);
	this->setDimension(dimension);
	this->setDataLoc(dataLoc);
}

template <class T>
template <class U>
T Field<T>::interpolate(const Vector<3, U>& loc) const
{
	if (dataLoc == Vertex)
		return gridInterpolate(loc);
	return cellInterpolate(loc);
}

template <class T>
Vector<3, int> Field<T>::gridDim() const
{
	if (dataLoc == Vertex)
		return dim;
	return dim + 1;
}

template <class T>
Vector<3, int> Field<T>::cellDim() const
{
	if (dataLoc == Center)
		return dim;
	return dim - 1;
}

template <class T>
template <class U>
T Field<T>::gridInterpolate(const Vector<3, U>& loc) const
{
	assert(loc.x() >= 0.0 && loc.y() >= 0.0 && loc.z() >= 0.0);
	assert(loc.x() <= T(cellDim().x()) && loc.y() <= T(cellDim().y()) && loc.z() <= T(cellDim().z()));
	Vector<3, U> coord;
	for (int i = 0; i < 3; ++i)
		coord[i] = std::min(loc[i], U(cellDim()[i] - std::numeric_limits<T>::epsilon()));
	return interpolate(loc, gridDim());
}

template <class T>
template <class U>
T Field<T>::cellInterpolate(const Vector<3, U>& gridLoc) const
{
	assert(gridLoc.x() >= 0.0 && gridLoc.y() >= 0.0 && gridLoc.z() >= 0.0);
	assert(gridLoc.x() <= T(cellDim().x()) && gridLoc.y() <= T(cellDim().y()) && gridLoc.z() <= T(cellDim().z()));
	Vector<3, U> loc = gridLoc - 0.5;
	for (int i = 0; i < 3; ++i)
	{
		loc[i] = std::max(loc[i], U(0.0));
		loc[i] = std::min(loc[i], U(cellDim()[i] - 1.0 - std::numeric_limits<T>::epsilon()));
	}
	return interpolate(loc, cellDim());
}

template <class T>
template <class U>
T Field<T>::interpolate(const Vector<3, U>& loc, const Vector<3, int>& dimension) const
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
	int index_000 = lower_bound[0] + dimension[0] * lower_bound[1] + dimension[0] * dimension[1] * lower_bound[2];
	int index_001 = lower_bound[0] + dimension[0] * lower_bound[1] + dimension[0] * dimension[1] * (lower_bound[2] + 1);
	int index_010 = lower_bound[0] + dimension[0] * (lower_bound[1] + 1) + dimension[0] * dimension[1] * lower_bound[2];
	int index_011 = lower_bound[0] + dimension[0] * (lower_bound[1] + 1) + dimension[0] * dimension[1] * (lower_bound[2] + 1);
	int index_100 = (lower_bound[0] + 1) + dimension[0] * lower_bound[1] + dimension[0] * dimension[1] * lower_bound[2];
	int index_101 = (lower_bound[0] + 1) + dimension[0] * lower_bound[1] + dimension[0] * dimension[1] * (lower_bound[2] + 1);
	int index_110 = (lower_bound[0] + 1) + dimension[0] * (lower_bound[1] + 1) + dimension[0] * dimension[1] * lower_bound[2];
	int index_111 = (lower_bound[0] + 1) + dimension[0] * (lower_bound[1] + 1) + dimension[0] * dimension[1] * (lower_bound[2] + 1);
	return field[index_000] * ratio_111 + field[index_001] * ratio_110
	     + field[index_010] * ratio_101 + field[index_011] * ratio_100
	     + field[index_100] * ratio_011 + field[index_101] * ratio_010
	     + field[index_110] * ratio_001 + field[index_111] * ratio_000;
}

#endif //__Field_h__