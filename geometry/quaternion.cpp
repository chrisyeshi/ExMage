#include "quaternion.h"

#include <cmath>

#include "vector.h"

Quaternion::Quaternion()
  : x(0.0), y(0.0), z(0.0), w(1.0)
{
}

Quaternion::Quaternion(float xx, float yy, float zz, float ww)
  : x(xx), y(yy), z(zz), w(ww)
{
}

void Quaternion::Normalize()
{
  float mag2 = Mag2();
  if (fabs(mag2) > 0.0001 && fabs(mag2 - 1.0) > 0.0001)
  {
    float mag = Mag();
    w /= mag;
    x /= mag;
    y /= mag;
    z /= mag;
  }
}

Quaternion Quaternion::Conjugate() const
{
  return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::operator*(const Quaternion& rq) const
{
  return Quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
                    w * rq.y + y * rq.w + z * rq.x - x * rq.z,
                    w * rq.z + z * rq.w + x * rq.y - y * rq.x,
                    w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}

Vector Quaternion::operator*(const Vector& vec) const
{
  Vector vn(vec);
  vn.normalize();

  Quaternion vecQuat, resQuat;
  vecQuat.x = vn.x;
  vecQuat.y = vn.y;
  vecQuat.z = vn.z;
  vecQuat.w = 0.0f;

  resQuat = vecQuat * Conjugate();
  resQuat = *this * resQuat;

  return (Vector(resQuat.x, resQuat.y, resQuat.z));
}

void Quaternion::FromAxis(const Vector& v, float angle)
{
  float sinAngle;
  angle *= 0.5f;
  Vector vn(v);
  vn.normalize();

  sinAngle = sin(angle);

  x = vn.x * sinAngle;
  y = vn.y * sinAngle;
  z = vn.z * sinAngle;
  w = cos(angle);
}

void Quaternion::FromEuler(float pitch, float yaw, float roll)
{
  float p = pitch * (M_PI / 180.0) / 2.0;
  float y = yaw * (M_PI / 180.0) / 2.0;
  float r = roll * (M_PI / 180.0) / 2.0;

  float sinp = sin(p);
  float siny = sin(y);
  float sinr = sin(r);
  float cosp = cos(p);
  float cosy = cos(y);
  float cosr = cos(r);

  this->x = sinr * cosp * cosy - cosr * sinp * siny;
  this->y = cosr * sinp * cosy + sinr * cosp * siny;
  this->z = cosr * cosp * siny - sinr * sinp * cosy;
  this->w = cosr * cosp * cosy + sinr * sinp * siny;

  Normalize();
}

void Quaternion::GetMatrix(float mat[16]) const
{
  float x2 = x * x;
  float y2 = y * y;
  float z2 = z * z;
  float xy = x * y;
  float xz = x * z;
  float yz = y * z;
  float wx = w * x;
  float wy = w * y;
  float wz = w * z;
 
  mat[0] = 1.0f - 2.0f * (y2 + z2);
  mat[1] = 2.0f * (xy - wz);
  mat[2] = 2.0f * (xz + wy);
  mat[3] = 0.0f;
  mat[4] = 2.0f * (xy + wz);
  mat[5] = 1.0f - 2.0f * (x2 + z2);
  mat[6] = 2.0f * (yz - wx);
  mat[7] = 0.0f;
  mat[8] = 2.0f * (xz - wy);
  mat[9] = 2.0f * (yz + wx);
  mat[10] = 1.0f - 2.0f * (x2 + y2);
  mat[11] = 0.0f;
  mat[12] = 0.0f;
  mat[13] = 0.0f;
  mat[14] = 0.0f;
  mat[15] = 1.0f;
}

void Quaternion::GetAxisAngle(Vector* axis, float* angle) const
{
  float scale = sqrt(x * x + y * y + z * z);
  axis->x = x / scale;
  axis->y = y / scale;
  axis->z = z / scale;
  *angle = acos(w) * 2.0f;
}

float Quaternion::Mag2() const
{
  return w * w + x * x + y * y + z * z;
}

float Quaternion::Mag() const
{
  return sqrt(Mag2());
}
