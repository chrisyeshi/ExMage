#include <cassert>
#include <cmath>

template <int Dim = 3, class T = float>
class Vector;

template <int Dim = 3, class T = float>
T dot(const Vector<Dim, T>& l, const Vector<Dim, T>& r);

template <int Dim = 3, class T = float>
T cross(const Vector<Dim, T>& l, const Vector<Dim, T>& r);

template <int Dim = 3, class T = float>
class Vector
{
public:
	Vector();

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
	// dot product
	friend T dot(const Vector<Dim, T>& l, const Vector<Dim, T>& r);
	// cross product
	friend Vector<Dim, T> cross(const Vector<Dim, T>& l, const Vector<Dim, T>& r);
	// length
	T length() const;
	// normalize
	Vector<Dim, T> normal() const;
	void normalize();

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
	ret[0] = l[1]r[2] - l[2]r[1];
	ret[1] = l[2]r[0] - l[0]r[2];
	ret[2] = l[0]r[1] - l[1]r[0];
	return ret;
}

template <int Dim, class T>
T length() const
{
	T sqsum = 0.0;
	for (int i = 0; i < Dim; ++i)
		sqsum += this->Vals[i] * this->Vals[i];
	return sqrt(sqsum);
}

template <int Dim, class T>
Vector<Dim, T> normal() const
{
	return *this / this->length();
}

template <int Dim, class T>
void normalize()
{
	T len = this->length();
	for (int i = 0; i < Dim; ++i)
		this->Vals[i] / len;
}