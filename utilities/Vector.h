#ifndef __Vector_h__
#define __Vector_h__

#include <cassert>
#include <cmath>
#include <iostream>

template <int Dim = 3, class T = float>
class Vector
{
public:
	Vector();
	Vector(const Vector<Dim, T>& v);
	Vector(const T& x, const T& y = 0.0, const T& z = 0.0);

	// convenient accessors
	T operator[](unsigned int idx) const;
	T& operator[](unsigned int idx);
	T x() const { return Vals[0]; }
	T& x() { return Vals[0]; }
	T y() const { return Vals[1]; }
	T& y() { return Vals[1]; }
	T z() const { return Vals[2]; }
	T& z() { return Vals[2]; }
	T w() const { return Vals[3]; }
	T& w() { return Vals[4]; }

	// operators
	//
	// =

	// 
	// "<<"" stream extraction operator is provided
	//
	// <
	bool operator<(const Vector<Dim, T>& right) const;
	bool operator>(const Vector<Dim, T>& right) const;
	// ==
	bool operator==(const Vector<Dim, T>& right) const;
	bool operator!=(const Vector<Dim, T>& right) const;
	// +
	Vector<Dim, T> operator+(const T& right) const;
	Vector<Dim, T> operator+(const Vector<Dim, T>& right) const;
	// -
	Vector<Dim, T> operator-(const T& right) const;
	Vector<Dim, T> operator-(const Vector<Dim, T>& right) const;
	// *
	Vector<Dim, T> operator*(const T& right) const;
	Vector<Dim, T> operator*(const Vector<Dim, T>& right) const;   // not dot product
	// /
	Vector<Dim, T> operator/(const T& right) const;
	Vector<Dim, T> operator/(const Vector<Dim, T>& right) const;
	// length
	T length2() const;
	T length() const;
	// normalize
	Vector<Dim, T> normal() const;
	void normalize();
	//
	// dot product and cross product are provided as external functions in this file.
	//
	// distance and distance2 are provided as external functions in this file.
	//

protected:

private:
	T Vals[Dim];
};

template <int Dim, class T>
Vector<Dim, T>::Vector()
{
	Vals[0] = Vals[1] = Vals[2] = 0.0;
}

template <int Dim, class T>
Vector<Dim, T>::Vector(const Vector<Dim, T>& v)
{
	for (int i = 0; i < Dim; ++i)
		this->Vals[i] = v[i];
}

template <int Dim, class T>
Vector<Dim, T>::Vector(const T& x, const T& y, const T& z)
{
	this->Vals[0] = x;
	if (Dim <= 1) return;
	this->Vals[1] = y;
	if (Dim <= 2) return;
	this->Vals[2] = z;
}

template <int Dim, class T>
T Vector<Dim, T>::operator[](unsigned int idx) const
{
	assert(idx < Dim);
	return Vals[idx];
}

template <int Dim, class T>
T& Vector<Dim, T>::operator[](unsigned int idx)
{
	assert(idx < Dim);
	return Vals[idx];
}

template <int Dim, class T>
std::ostream& operator<<(std::ostream& os, const Vector<Dim, T>& v)
{
	os << '[';
	for (int i = 0; i < Dim - 1; ++i)
		os << v[i] << ',';
	os << v[Dim - 1] << ']';
	return os;
}

template <int Dim, class T>
bool Vector<Dim, T>::operator<(const Vector<Dim, T>& right) const
{
	for (int i = Dim - 1; i >= 0; --i)
	{
		if (this->Vals[i] - right[i] < -0.0001)
			return true;
		else if (this->Vals[i] - right[i] > 0.0001)
			return false;
	}
	return false;
}

template <int Dim, class T>
bool Vector<Dim, T>::operator>(const Vector<Dim, T>& right) const
{
	return right < *this;
}

template <int Dim, class T>
bool Vector<Dim, T>::operator==(const Vector<Dim, T>& right) const
{
	return !(*this < right) && !(*this > right);
}

template <int Dim, class T>
bool Vector<Dim, T>::operator!=(const Vector<Dim, T>& right) const
{
	return !(*this == right);
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator+(const T& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] + right;
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator+(const Vector<Dim, T>& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] + right[i];
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator-(const T& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] - right;
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator-(const Vector<Dim, T>& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] - right[i];
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator*(const T& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] * right;
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator*(const Vector<Dim, T>& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] * right[i];
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator/(const T& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] / right;
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::operator/(const Vector<Dim, T>& right) const
{
	Vector<Dim, T> ret;
	for (int i = 0; i < Dim; ++i)
		ret[i] = this->Vals[i] / right[i];
	return ret;
}

template <int Dim, class T>
T Vector<Dim, T>::length2() const
{
	T sqsum = 0.0;
	for (int i = 0; i < Dim; ++i)
		sqsum += this->Vals[i] * this->Vals[i];
	return sqsum;
}

template <int Dim, class T>
T Vector<Dim, T>::length() const
{
	return sqrt(length2());
}

template <int Dim, class T>
Vector<Dim, T> Vector<Dim, T>::normal() const
{
	return *this / this->length();
}

template <int Dim, class T>
void Vector<Dim, T>::normalize()
{
	T len = this->length();
	for (int i = 0; i < Dim; ++i)
		this-> Vals[i] = this->Vals[i] / len;
}

template <int Dim, class T>
T dot(const Vector<Dim, T>& l, const Vector<Dim, T>& r)
{
	T ret = 0.0;
	for (int i = 0; i < Dim; ++i)
		ret += l[i] * r[i];
	return ret;
}

template <int Dim, class T>
Vector<Dim, T> cross(const Vector<Dim, T>& l, const Vector<Dim, T>& r)
{
	assert(Dim == 3);
	Vector<Dim, T> ret;
	ret[0] = l[1] * r[2] - l[2] * r[1];
	ret[1] = l[2] * r[0] - l[0] * r[2];
	ret[2] = l[0] * r[1] - l[1] * r[0];
	return ret;
}

template <int Dim, class T>
T distance2(const Vector<Dim, T>& l, const Vector<Dim, T>& r)
{
	T sum2 = 0.0;
	Vector<Dim, T> delta = l - r;
	return delta.length2();
}

template <int Dim, class T>
T distance(const Vector<Dim, T>& l, const Vector<Dim, T>& r)
{
	return sqrt(distance2(l, r));
}

#endif //__Vector_h__