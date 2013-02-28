#include "ConfigReader.h"

#include <sstream>

ConfigReader::ConfigReader()
{
  TimeStepRange[0] = 1;
  TimeStepRange[1] = 20;
}

void ConfigReader::GetTimeStepRange(int time_step_range[2]) const
{
  time_step_range[0] = TimeStepRange[0];
  time_step_range[1] = TimeStepRange[1];
}

bool ConfigReader::Read()
{
  fin.open(FileName.c_str());
  if (!fin.good())
    return false;

  while (fin.good())
  {
    std::string line;
    std::getline(fin, line);

    if (line.empty())
      continue;

    if (line[0] == '#')
      continue;

    if (line == "camera")
    {
      if (!readCamera()) return false;
      continue;
    }

    if (line == "light position")
    {
      if (!readLightPosition()) return false;
      continue;
    }

    if (line == "time step range")
    {
      if (!readTimeStepRange()) return false;
      continue;
    }

    if (line == "read root")
    {
      if (!readReadRoot()) return false;
      continue;
    }
  }

  fin.close();
  return true;
}

bool ConfigReader::readCamera()
{
  CameraCore cam;
  if (!readPoint(&cam.Position)) return false;
  if (!readPoint(&cam.Focal)) return false;
  if (!readVector(&cam.ViewUp)) return false;
  Cameras.push_back(cam);
  return true;
}

bool ConfigReader::readLightPosition()
{
  if (!readPoint(&LightPosition)) return false;
  return true;
}

bool ConfigReader::readTimeStepRange()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> TimeStepRange[0] >> TimeStepRange[1];
  return true;
}

bool ConfigReader::readReadRoot()
{
  if (!fin.good())
    return false;
  std::getline(fin, ReadRoot);
  return true;
}

bool ConfigReader::readPoint(Point* pt)
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> pt->x >> pt->y >> pt->z;
  return true;
}

bool ConfigReader::readVector(Vector* vec)
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> vec->x >> vec->y >> vec->z;
  return true;
}
