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

#endif
