#ifndef __CameraCore_h
#define __CameraCore_h

#include "Geometry/point.h"
#include "Geometry/vector.h"

class CameraCore
{
public:
  Point Position;
  Point Focal;
  Vector ViewUp;
};

#endif
