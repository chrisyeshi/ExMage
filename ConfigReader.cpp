#include "ConfigReader.h"

#include <sstream>

ConfigReader::ConfigReader()
{
  TimeStepRange[0] = 1;
  TimeStepRange[1] = 20;
}

ConfigReader::ConfigReader(const std::string filename)
{
  TimeStepRange[0] = 1;
  TimeStepRange[1] = 20;
  FileName = filename;
  this->Read();
}

void ConfigReader::GetTimeStepRange(int time_step_range[2]) const
{
  time_step_range[0] = TimeStepRange[0];
  time_step_range[1] = TimeStepRange[1];
}

void ConfigReader::GetTotalSize(int total_size[3]) const
{
  for (int i = 0; i < 3; ++i)
    total_size[i] = TotalSize[i];
}

void ConfigReader::GetResolution(int resolution[2]) const
{
  for (int i = 0; i < 2; ++i)
    resolution[i] = Resolution[i];
}

void ConfigReader::GetRegionCount(int region_count[3]) const
{
  for (int i = 0; i < 3; ++i)
    region_count[i] = RegionCount[i];
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

    if (line == "out root")
    {
      if (!readOutRoot()) return false;
      continue;
    }

    if (line == "total size")
    {
      if (!readTotalSize()) return false;
      continue;
    }

    if (line == "region count")
    {
      if (!readRegionCount()) return false;
      continue;
    }

    if (line == "region particle count")
    {
      if (!readRegionParticleCount()) return false;
      continue;
    }

    if (line == "velocity")
    {
      if (!readVelocity()) return false;
      continue;
    }

    if (line == "resolution")
    {
      if (!readResolution()) return false;
      continue;
    }

    if (line == "tube radius")
    {
      if (!readTubeRadius()) return false;
      continue;
    }

    if (line == "max particle gap")
    {
      if (!readMaxParticleGap()) return false;
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

bool ConfigReader::readOutRoot()
{
  if (!fin.good())
    return false;
  std::getline(fin, OutRoot);
  return true;
}

bool ConfigReader::readTotalSize()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> TotalSize[0] >> TotalSize[1] >> TotalSize[2];
  return true;
}

bool ConfigReader::readRegionCount()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> RegionCount[0] >> RegionCount[1] >> RegionCount[2];
  return true;
}

bool ConfigReader::readRegionParticleCount()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> RegionParticleCount;
  return true;
}

bool ConfigReader::readVelocity()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> Velocity;
  return true;
}

bool ConfigReader::readResolution()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> Resolution[0] >> Resolution[1];
  return true;
}

bool ConfigReader::readTubeRadius()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> TubeRadius;
  return true;
}

bool ConfigReader::readMaxParticleGap()
{
  if (!fin.good())
    return false;
  std::string line;
  std::getline(fin, line);
  std::stringstream oss;
  oss.str(line);
  oss >> MaxParticleGap;
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
