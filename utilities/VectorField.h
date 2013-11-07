#ifndef __VectorField_h__
#define __VectorField_h__

// The first "vDim" fields are the velocities (x, y, z, ...)
// The rest are scalar fields
//
// This class doesn't free memories either.

#include <vector>

#include "Field.h"
#include "Vector.h"

template <class T = float>
class VectorField
{
private:
	const static int vDim = 3;

public:
	VectorField();

	// setters
	void set(const std::vector<Field<T> >& fields) { this->fields = fields; }
	void set(const std::vector<T*>& fields, int x, int y, int z);
	void set(const std::vector<T*>& fields, const std::vector<int>& dimension);
	void set(const std::vector<T*>& fields, int* dimension);
	void set(T** fields, int nFields, int x, int y, int z);
	void set(T** fields, int nFields, const std::vector<int>& dimension);
	void set(T** fields, int nFields, int* dimension);

	void setVelocities(const std::vector<Field<T> >& velocities);
	void setVelocities(const std::vector<T*>& velocities, int x, int y, int z);
	void setVelocities(const std::vector<T*>& velocities, const std::vector<int>& dimension);
	void setVelocities(const std::vector<T*>& velocities, int* dimension);
	void setVelocities(T** velocities, int x, int y, int z);
	void setVelocities(T** velocities, const std::vector<int>& dimension);
	void setVelocities(T** velocities, int* dimension);

	void setScalars(const std::vector<Field<T> >& scalars);
	void setScalars(const std::vector<T*>& scalars, int x, int y, int z);
	void setScalars(const std::vector<T*>& scalars, const std::vector<int>& dimension);
	void setScalars(const std::vector<T*>& scalars, int* dimension);
	void setScalars(T** scalars, int nFields, int x, int y, int z);
	void setScalars(T** scalars, int nFields, const std::vector<int>& dimension);
	void setScalars(T** scalars, int nFields, int* dimension);

	void addScalar(T* field, int x, int y, int z);
	void addScalar(T* field, const std::vector<int>& dimension);
	void addScalar(T* field, int* dimension);

	// accessors
	template <class U>
	Vector<3, T> getVelocity(const Vector<vDim, U>& loc) const;
	template <class U>
	std::vector<T> getScalars(const Vector<vDim, U>& loc) const;
	template <class U>
	std::vector<T> get(const Vector<vDim, U>& loc) const ;

protected:
	unsigned int nScalars() const { return fields.size() - vDim; }

private:
	std::vector<Field<T> > fields;
};

template <class T>
VectorField<T>::VectorField() : fields(vDim)
{}

template <class T>
void VectorField<T>::set(const std::vector<T*>& fields, int x, int y, int z)
{
	std::vector<Field<T> > fFields(fields.size());
	for (unsigned int i = 0; i < fields.size(); ++i)
		fFields[i] = Field<T>(fields[i], x, y, z);
	this->set(fFields);
}

template <class T>
void VectorField<T>::set(const std::vector<T*>& fields, const std::vector<int>& dimension)
{
	this->set(fields, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::set(const std::vector<T*>& fields, int* dimension)
{
	this->set(fields, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::set(T** fields, int nFields, int x, int y, int z)
{
	std::vector<T*> vFields(nFields);
	for (int i = 0; i < nFields; ++i)
		vFields[i] = fields[i];
	this->set(vFields, x, y, z);
}

template <class T>
void VectorField<T>::set(T** fields, int nFields, const std::vector<int>& dimension)
{
	this->set(fields, nFields, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::set(T** fields, int nFields, int* dimension)
{
	this->set(fields, nFields, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setVelocities(const std::vector<Field<T> >& velocities)
{
	std::copy(velocities.begin(), velocities.end(), fields.begin());
}

template <class T>
void VectorField<T>::setVelocities(const std::vector<T*>& velocities, int x, int y, int z)
{
	std::vector<Field<T> > fVelocities(velocities.size());
	for (unsigned int i = 0; i < velocities.size(); ++i)
		fVelocities[i] = Field<T>(velocities[i], x, y, z);
	this->setVelocities(fVelocities);
}

template <class T>
void VectorField<T>::setVelocities(const std::vector<T*>& velocities, const std::vector<int>& dimension)
{
	this->setVelocities(velocities, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setVelocities(const std::vector<T*>& velocities, int* dimension)
{
	this->setVelocities(velocities, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setVelocities(T** velocities, int x, int y, int z)
{
	std::vector<T*> vVelocities(vDim);
	for (unsigned int i = 0; i < vDim; ++i)
		vVelocities[i] = velocities[i];
	this->setVelocities(vVelocities, x, y, z);
}

template <class T>
void VectorField<T>::setVelocities(T** velocities, const std::vector<int>& dimension)
{
	this->setVelocities(velocities, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setVelocities(T** velocities, int* dimension)
{
	this->setVelocities(velocities, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setScalars(const std::vector<Field<T> >& scalars)
{
	fields.resize(vDim);
	fields.insert(fields.end(), scalars.begin(), scalars.end());
}

template <class T>
void VectorField<T>::setScalars(const std::vector<T*>& scalars, int x, int y, int z)
{
	std::vector<Field<T> > fScalars(scalars.size());
	for (unsigned int i = 0; i < scalars.size(); ++i)
		fScalars[i] = Field<T>(scalars[i], x, y, z);
	this->setScalars(fScalars);
}

template <class T>
void VectorField<T>::setScalars(const std::vector<T*>& scalars, const std::vector<int>& dimension)
{
	this->setScalars(scalars, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setScalars(const std::vector<T*>& scalars, int* dimension)
{
	this->setScalars(scalars, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setScalars(T** scalars, int nFields, int x, int y, int z)
{
	std::vector<T*> vScalars(nFields);
	for (unsigned int i = 0; i < vScalars.size(); ++i)
		vScalars[i] = scalars[i];
	this->setScalars(vScalars, x, y, z);
}

template <class T>
void VectorField<T>::setScalars(T** scalars, int nFields, const std::vector<int>& dimension)
{
	this->setScalars(scalars, nFields, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::setScalars(T** scalars, int nFields, int* dimension)
{
	this->setScalars(scalars, nFields, dimension[0], dimension[1], dimension[2]);
}

template <class T>
void VectorField<T>::addScalar(T* field, int x, int y, int z)
{
	fields.push_back(Field<T>(field, x, y, z));
}

template <class T>
void VectorField<T>::addScalar(T* field, const std::vector<int>& dimension)
{
	fields.push_back(Field<T>(field, dimension));
}

template <class T>
void VectorField<T>::addScalar(T* field, int* dimension)
{
	fields.push_back(Field<T>(field, dimension));
}

template <class T>
template <class U>
Vector<3, T> VectorField<T>::getVelocity(const Vector<3, U>& loc) const
{
	assert(fields.size() >= vDim);
	Vector<vDim, T> velocity;
	for (unsigned int i = 0; i < vDim; ++i)
		velocity[i] = fields[i].interpolate(loc);
	return velocity;
}

template <class T>
template <class U>
std::vector<T> VectorField<T>::getScalars(const Vector<3, U>& loc) const
{
	std::vector<T> ret(nScalars());
	for (unsigned int i = 0; i < nScalars(); ++i)
		ret[i] = fields[i + vDim].interpolate(loc);
	return ret;
}

template <class T>
template <class U>
std::vector<T> VectorField<T>::get(const Vector<3, U>& loc) const
{
	std::vector<T> ret(fields.size());
	for (unsigned int i = 0; i < fields.size(); ++i)
		ret[i] = fields[i].interpolate(loc);
	return ret;
}

#endif //__VectorField_h__