#ifndef __ConfigReader_h
#define __ConfigReader_h

#include <string>
#include "./picojson.h"
#include "CameraCore.h"
#include "Vector.h"

class ConfigReader
{
public:
    // Singleton
    static ConfigReader& getInstance()
    {
        static ConfigReader instance(filename);
        return instance;
    }
    static void setFile(const std::string& fn)
    {
        filename = fn;
    }

    std::vector<CameraCore> GetCameras() const;
    Vector<> GetLightPosition() const;
    std::vector<int> GetTimeStepRange() const;
    std::string GetReadRoot() const;
    std::string GetOutRoot() const;
    std::vector<int> GetTotalSize() const;
    std::vector<int> GetRegionCount() const;
    int GetRegionParticleCount() const;
    double GetVelocity() const;
    std::vector<int> GetResolution() const;
    double GetTubeRadius() const;
    double GetMaxParticleGap() const;
    std::vector<std::string> GetInputAttributes() const;
    std::vector<std::string> GetOutputAttributes() const;
    int GetAttributeCount() const;
    std::string GetFileFormat() const;

private:
    static std::string filename;
    picojson::value v;

    ConfigReader(); // Don't implement
    ConfigReader(const std::string& filename);
    ConfigReader(const ConfigReader&);      // Don't implement
    void operator=(const ConfigReader&);    // Don't implement
};

/*
#include <vector>
#include <string>
#include <fstream>

#include "CameraCore.h"
#include "Geometry/point.h"

class ConfigReader
{
public:
  ConfigReader(); // need to SetFileName() and then envoke Read()
  ConfigReader(const std::string filename); // this also reads the file
  ~ConfigReader() {};

  void SetFileName(const std::string& filename) {FileName = filename;};
  bool Read();

  std::vector<CameraCore> GetCameras() const {return Cameras;};
  Point GetLightPosition() const {return LightPosition;};
  void GetTimeStepRange(int time_step_range[2]) const;
  const int* GetTimeStepRange() const {return TimeStepRange;};
  const std::string& GetReadRoot() const {return ReadRoot;};
  const std::string& GetOutRoot() const {return OutRoot;};
  void GetTotalSize(int total_size[3]) const;
  const int* GetTotalSize() const {return TotalSize;};
  void GetRegionCount(int region_count[3]) const;
  const int* GetRegionCount() const {return RegionCount;};
  int GetRegionParticleCount() const {return RegionParticleCount;};
  float GetVelocity() const {return Velocity;};
  void GetResolution(int resolution[2]) const;
  const int* GetResolution() const {return Resolution;};
  float GetTubeRadius() const {return TubeRadius;};
  float GetMaxParticleGap() const {return MaxParticleGap;};
  const std::vector<std::string>& GetInputAttributes() const {return InputAttributes;}
  const std::vector<std::string>& GetOutputAttributes() const {return OutputAttributes;}
  const std::string& GetFileFormat() const {return FileFormat;}

protected:
  std::fstream fin;
  std::string FileName;
  std::vector<CameraCore> Cameras;
  Point LightPosition;
  int TimeStepRange[2];
  std::string ReadRoot;
  std::string OutRoot;
  int TotalSize[3];
  int RegionCount[3];
  int RegionParticleCount;
  float Velocity;
  int Resolution[2];
  float TubeRadius;
  float MaxParticleGap;
  std::vector<std::string> InputAttributes;
  std::vector<std::string> OutputAttributes;
  std::string FileFormat;

  bool readCamera();
  bool readLightPosition();
  bool readTimeStepRange();
  bool readReadRoot();
  bool readOutRoot();
  bool readTotalSize();
  bool readRegionCount();
  bool readRegionParticleCount();
  bool readVelocity();
  bool readResolution();
  bool readTubeRadius();
  bool readMaxParticleGap();
  bool readInputAttributes();
  bool readOutputAttributes();
  bool readFileFormat();

  bool readPoint(Point* pt);
  bool readVector(Vector* vec);
};
*/

#endif
