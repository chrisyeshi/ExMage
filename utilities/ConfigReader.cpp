#include "ConfigReader.h"

#include <fstream>

std::string ConfigReader::filename = "configure.json";

ConfigReader::ConfigReader(const std::string& filename)
{
    std::ifstream fin(filename.c_str());
    fin >> v;
}

std::vector<CameraCore> ConfigReader::GetCameras() const
{
    std::vector<CameraCore> ret;

    picojson::value camera = v.get("camera");
    if (camera.is<picojson::null>())
    {
        std::cout << "Warning: No Camera Found!" << std::endl;
        return ret;
    }
    picojson::array campos = camera.get("position").get<picojson::array>();
    picojson::array camfoc = camera.get("focal").get<picojson::array>();
    picojson::array viewup = camera.get("up").get<picojson::array>();

    Point camPos;
    camPos.x = campos[0].get<double>();
    camPos.y = campos[1].get<double>();
    camPos.z = campos[2].get<double>();

    Point camFoc;
    camFoc.x = camfoc[0].get<double>();
    camFoc.y = camfoc[1].get<double>();
    camFoc.z = camfoc[2].get<double>();

    Vector viewUp;
    viewUp.x = viewup[0].get<double>();
    viewUp.y = viewup[1].get<double>();
    viewUp.z = viewup[2].get<double>();

    CameraCore cam;
    cam.Position = camPos;
    cam.Focal = camFoc;
    cam.ViewUp = viewUp;

    ret.push_back(cam);
    return ret;
}

Point ConfigReader::GetLightPosition() const
{
    if (!v.contains("light position"))
    {
        std::cout << "Warning: Using default light position!" << std::endl;
        Point empty;
        return empty;
    }

    Point ret;
    picojson::array litposa = v.get("light position").get<picojson::array>();
    ret.x = litposa[0].get<double>();
    ret.y = litposa[1].get<double>();
    ret.z = litposa[2].get<double>();
    
    return ret;
}

std::vector<int> ConfigReader::GetTimeStepRange() const
{
    if (!v.contains("time step range"))
    {
        std::cout << "Warning: No time step range found!" << std::endl;
        std::vector<int> ret(2);
        ret[0] = ret[1] = 0;
        return ret;
    }

    std::vector<int> ret(2);
    picojson::array tsa = v.get("time step range").get<picojson::array>();
    ret[0] = tsa[0].get<double>();
    ret[1] = tsa[1].get<double>();

    return ret;
}

std::string ConfigReader::GetReadRoot() const
{
    if (!v.contains("read root"))
    {
        std::cout << "Warning: Using the current directory as the read root!" << std::endl;
        return "";
    }

    return v.get("read root").get<std::string>();
}

std::string ConfigReader::GetOutRoot() const
{
    if (!v.contains("out root"))
    {
        std::cout << "Warning: Using the current/output directory as the output directory!" << std::endl;
        return "output";
    }

    return v.get("out root").get<std::string>();
}

std::vector<int> ConfigReader::GetTotalSize() const
{
    if (!v.contains("total size"))
    {
        std::cout << "Warning: Missing total size!" << std::endl;
        std::vector<int> ret(3);
        return ret;
    }

    picojson::array tsa = v.get("total size").get<picojson::array>();
    std::vector<int> ret(3);
    for (unsigned int i = 0; i < ret.size(); ++i)
        ret[i] = tsa[i].get<double>();

    return ret;
}

std::vector<int> ConfigReader::GetRegionCount() const
{
    if (!v.contains("region count"))
    {
        std::cout << "Warning: Missing region count!" << std::endl;
        std::vector<int> ret(3);
        ret[0] = ret[1] = ret[2] = 2;
        return ret;
    }

    picojson::array rca = v.get("region count").get<picojson::array>();
    std::vector<int> ret(3);
    for (unsigned int i = 0; i < ret.size(); ++i)
        ret[i] = rca[i].get<double>();

    return ret;
}

int ConfigReader::GetRegionParticleCount() const
{
    if (!v.contains("region particle count"))
    {
        std::cout << "Warning: Using default region particle count!" << std::endl;
        return 1;
    }

    return v.get("region particle count").get<double>();
}

double ConfigReader::GetVelocity() const
{
    if (!v.contains("velocity"))
    {
        std::cout << "Warning: Using default velocity!" << std::endl;
        return 1.0;
    }

    return v.get("velocity").get<double>();
}

std::vector<int> ConfigReader::GetResolution() const
{
    if (!v.contains("resolution"))
    {
        std::cout << "Warning: Using default resolution, 512x512!" << std::endl;
        std::vector<int> ret(2);
        ret[0] = ret[1] = 512;
        return ret;
    }

    picojson::array ra = v.get("resolution").get<picojson::array>();
    std::vector<int> ret(2);
    for (unsigned int i = 0; i < ret.size(); ++i)
        ret[i] = ra[i].get<double>();

    return ret;
}

double ConfigReader::GetTubeRadius() const
{
    if (!v.contains("tube radius"))
    {
        std::cout << "Warning: Using default tube radius, 1.0!" << std::endl;
        return 1.0;
    }

    return v.get("tube radius").get<double>();
}

double ConfigReader::GetMaxParticleGap() const
{
    if (!v.contains("max particle gap"))
    {
        std::cout << "Warning: Using 2xtube radius as max particle gap!" << std::endl;
        return 2.0 * GetTubeRadius();
    }

    return v.get("max particle gap").get<double>();
}

std::vector<std::string> ConfigReader::GetInputAttributes() const
{
    if (!v.contains("input attributes"))
    {
        std::cout << "Warning: No input attributes!" << std::endl;
        return std::vector<std::string>();
    }

    picojson::array iaa = v.get("input attributes").get<picojson::array>();
    std::vector<std::string> ret(iaa.size());
    for (unsigned int i = 0; i < iaa.size(); ++i)
        ret[i] = iaa[i].get<std::string>();

    return ret;
}

std::vector<std::string> ConfigReader::GetOutputAttributes() const
{
    if (!v.contains("output attributes"))
    {
        std::cout << "Warning: No output attributes!" << std::endl;
        return std::vector<std::string>();
    }

    picojson::array oaa = v.get("output attributes").get<picojson::array>();
    std::vector<std::string> ret(oaa.size());
    for (unsigned int i = 0; i < oaa.size(); ++i)
        ret[i] = oaa[i].get<std::string>();

    return ret;
}

int ConfigReader::GetAttributeCount() const
{
    if (!v.contains("input attributes"))
    {
        std::cout << "Warning: No input attributes!" << std::endl;
        return 0;
    }

    return v.get("input attributes").get<picojson::array>().size();
}

std::string ConfigReader::GetFileFormat() const
{
    if (!v.contains("file format"))
    {
        std::cout << "Warning: Using raw has default file format!" << std::endl;
        return "raw";
    }

    return v.get("file format").get<std::string>();
}

/*
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

    if (line == "input attributes")
    {
      if (!readInputAttributes()) return false;
      continue;
    }

    if (line == "output attributes")
    {
      if (!readOutputAttributes()) return false;
      continue;
    }

    if (line == "file format")
    {
      if (!readFileFormat()) return false;
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

bool ConfigReader::readInputAttributes()
{
  if (!fin.good())
    return false;
  std::string line;
  while (std::getline(fin, line))
  {
    if (line == "") break;
    InputAttributes.push_back(line);
  }
  
  return true;
}

bool ConfigReader::readOutputAttributes()
{
  if (!fin.good())
    return false;
  std::string line;
  while (std::getline(fin, line))
  {
    if (line == "") break;
    OutputAttributes.push_back(line);
  }
  
  return true;
}

bool ConfigReader::readFileFormat()
{
  if (!fin.good())
    return false;
  std::getline(fin, FileFormat);
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
*/
