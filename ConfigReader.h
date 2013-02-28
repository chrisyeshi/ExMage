#ifndef __ConfigReader_h
#define __ConfigReader_h

#include <vector>
#include <string>
#include <fstream>

#include "CameraCore.h"
#include "Geometry/point.h"

class ConfigReader
{
public:
  ConfigReader();
  ~ConfigReader() {};

  void SetFileName(const std::string& filename) {FileName = filename;};
  bool Read();

  std::vector<CameraCore> GetCameras() const {return Cameras;};
  Point GetLightPosition() const {return LightPosition;};
  void GetTimeStepRange(int time_step_range[2]) const;
  const int* GetTimeStepRange() const {return TimeStepRange;};
  std::string GetReadRoot() const {return ReadRoot;};

protected:
  std::string FileName;
  std::vector<CameraCore> Cameras;
  Point LightPosition;
  int TimeStepRange[2];
  std::string ReadRoot;
  std::fstream fin;

  bool readCamera();
  bool readLightPosition();
  bool readTimeStepRange();
  bool readReadRoot();

  bool readPoint(Point* pt);
  bool readVector(Vector* vec);
};

#endif
