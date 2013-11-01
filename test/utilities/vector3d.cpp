#include "Vector.h"

#include <cassert>
#include <iostream>

int main(void)
{
	// accessors
	Vector<3> v;
	v[0] = 1.f;
	v[1] = 1.f;
	v[2] = 1.f;
	std::cout << "testing the accessors..." << std::endl;
	assert(fabs(v.x() - 1.f) < 0.0001);
	assert(fabs(v.y() - 1.f) < 0.0001);
	assert(fabs(v.z() - 1.f) < 0.0001);
	std::cout << "accessors working!" << std::endl;

	Vector<3> v1, v2, vr, vc;
	v1[0] = v1[1] = v1[2] = v2[0] = v2[1] = v2[2] = 1.f;
	// operator==
	assert(v1 == v2);
	// operator!=
	vr = v1 + v2;
	assert(v1 != vr);
	// operator+
	vr = v1 + 1.f;
	assert(vr == Vector<3>(2.f, 2.f, 2.f));
	vr = v1 + v2;
	assert(vr == Vector<3>(2.f, 2.f, 2.f));
	// operator-
	vr = v1 - 1.f;
	assert(vr == Vector<3>(0.f, 0.f, 0.f));
	vr = v1 - v2;
	assert(vr == Vector<3>(0.f, 0.f, 0.f));
	// operator*
	vr = v1 * 1.f;
	assert(vr == Vector<3>(1.f, 1.f, 1.f));
	vr = v1 * v2;
	assert(vr == Vector<3>(1.f, 1.f, 1.f));
	// operator/
	vr = v1 / 1.f;
	assert(vr == Vector<3>(1.f, 1.f, 1.f));
	vr = v1 / v2;
	assert(vr == Vector<3>(1.f, 1.f, 1.f));
	// dot
	float d = dot(v1, v2);
	assert(fabs(d - 3.f) < 0.0001);
	// cross
	vr = cross(Vector<3>(0.f, 1.f, 0.f), Vector<3>(1.f, 0.f, 0.f));
	assert(vr == Vector<3>(0.f, 0.f, -1.f));
	// length
	float l = vr.length();
	assert(fabs(l - 1.f) < 0.0001);
	// normal
	Vector<3> n = vr.normal();
	vr.normalize();
	assert(n == vr);
	assert(n == Vector<3>(0.f, 0.f, -1.f));
	// normalize
	Vector<3> three(3.f, 0.f, 0.f);
	three.normalize();
	assert(three == Vector<3>(1.f, 0.f, 0.f));
	// distance
	float dist = distance(Vector<3>(0.f, 0.f, 0.f), Vector<3>(1.f, 0.f, 0.f));
	assert(fabs(dist - 1.f) < 0.0001);
	std::cout << "all working!" << std::endl;

	return 0;
}