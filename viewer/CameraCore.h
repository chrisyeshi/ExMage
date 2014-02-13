#ifndef __CameraCore_h
#define __CameraCore_h

#include "point.h"
#include "vector.h"

class CameraCore
{
public:
  Point Position;
  Point Focal;
  Vector ViewUp;

  void pan(const Vector& vec);
};

#endif
