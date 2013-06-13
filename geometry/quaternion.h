#ifndef __Quaternion_h
#define __Quaternion_h

class Vector;

class Quaternion
{
public:
  Quaternion();
  Quaternion(float xx, float yy, float zz, float ww);
  ~Quaternion() {};

  float x, y, z, w;

  void Normalize();
  Quaternion Conjugate() const; // remember to normalize first for this to be correct
  Quaternion operator*(const Quaternion& rq) const;
  Vector operator*(const Vector& vec) const;
  void FromAxis(const Vector& v, float angle); // radians
  void FromEuler(float pitch, float yaw, float roll);
  void GetMatrix(float mat[16]) const;
  void GetAxisAngle(Vector* axis, float* angle) const;
  float Mag2() const;
  float Mag() const;
};

#endif
