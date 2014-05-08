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
	void set(const std::vector<T*>& fields, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);
	void set(T** fields, int nFields, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);

	void setVelocities(const std::vector<Field<T> >& velocities);
	void setVelocities(const std::vector<T*>& velocities, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);
	void setVelocities(T** velocities, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);

	void setScalars(const std::vector<Field<T> >& scalars);
	void setScalars(const std::vector<T*>& scalars, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);
	void setScalars(T** scalars, int nFields, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);

	void addScalar(T* field, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc = Field<T>::Center);

	// accessors
	template <class U>
	Vector<3, T> getVelocity(const Vector<vDim, U>& loc) const;
	template <class U>
	std::vector<T> getScalars(const Vector<vDim, U>& loc) const;
	template <class U>
	std::vector<T> get(const Vector<vDim, U>& loc) const ;
	unsigned int nVelocities() const { return vDim; }
	unsigned int nScalars() const { return fields.size() - vDim; }
	unsigned int nFields() const { return nVelocities() + nScalars(); }

protected:

private:
	std::vector<Field<T> > fields;
};

template <class T>
VectorField<T>::VectorField() : fields(vDim)
{
}

template <class T>
void VectorField<T>::set(const std::vector<T*>& fields, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	std::vector<Field<T> > fFields(fields.size());
	for (unsigned int i = 0; i < fields.size(); ++i)
		fFields[i] = Field<T>(fields[i], dimension, dataLoc);
	this->set(fFields);
}

template <class T>
void VectorField<T>::set(T** fields, int nFields, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	std::vector<T*> vFields(nFields);
	for (int i = 0; i < nFields; ++i)
		vFields[i] = fields[i];
	this->set(vFields, dimension, dataLoc);
}

template <class T>
void VectorField<T>::setVelocities(const std::vector<Field<T> >& velocities)
{
	std::copy(velocities.begin(), velocities.end(), fields.begin());
}

template <class T>
void VectorField<T>::setVelocities(const std::vector<T*>& velocities, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	std::vector<Field<T> > fVelocities(velocities.size());
	for (unsigned int i = 0; i < velocities.size(); ++i)
		fVelocities[i] = Field<T>(velocities[i], dimension, dataLoc);
	this->setVelocities(fVelocities);
}

template <class T>
void VectorField<T>::setVelocities(T** velocities, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	std::vector<T*> vVelocities(vDim);
	for (unsigned int i = 0; i < vDim; ++i)
		vVelocities[i] = velocities[i];
	this->setVelocities(vVelocities, dimension, dataLoc);
}

template <class T>
void VectorField<T>::setScalars(const std::vector<Field<T> >& scalars)
{
	fields.resize(vDim);
	fields.insert(fields.end(), scalars.begin(), scalars.end());
}

template <class T>
void VectorField<T>::setScalars(const std::vector<T*>& scalars, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	std::vector<Field<T> > fScalars(scalars.size());
	for (unsigned int i = 0; i < scalars.size(); ++i)
		fScalars[i] = Field<T>(scalars[i], dimension, dataLoc);
	this->setScalars(fScalars);
}

template <class T>
void VectorField<T>::setScalars(T** scalars, int nFields, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	std::vector<T*> vScalars(nFields);
	for (unsigned int i = 0; i < vScalars.size(); ++i)
		vScalars[i] = scalars[i];
	this->setScalars(vScalars, dimension, dataLoc);
}

template <class T>
void VectorField<T>::addScalar(T* field, const Vector<3, int> dimension, typename Field<T>::DataLoc dataLoc)
{
	fields.push_back(Field<T>(field, dimension, dataLoc));
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