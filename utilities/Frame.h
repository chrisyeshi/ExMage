#ifndef __Frame_h
#define __Frame_h

#include <string>
#include <vector>

#include "CameraCore.h"

class Frame
{
public:
  Frame();
  ~Frame();

  void Reset();

  float* GetDepthMap() const {return DepthMap;};
  unsigned char* GetColorMap() const {return ColorMap;};
  unsigned char* GetNormalMap() const {return NormalMap;};
  float* GetScalarMap(const int index) const;

  void SetDepthMap(const float* depth);
  void SetColorMap(const unsigned char* color);
  void SetNormalMap(const unsigned char* normal);
  void SetNumberOfScalarMaps(const int num);
  int GetNumberOfScalarMaps() const {return ScalarMaps.size();};
  void SetScalarMap(const int index, const float* scalar);

  void SetCamera(const CameraCore& camera) {Camera = camera;};
  CameraCore GetCamera() const {return Camera;};
  void SetSize(int width, int height);
  void SetSize(int size[2]);
  void GetSize(int size[2]) const {size[0] = Size[0]; size[1] = Size[1];};
  void GetDataDomain(double domain[6]) const;
  void SetDataDomain(double domain[6]);
  void SetFileName(const std::string filename) {FileName = filename;};
  bool Read();
  bool Write();

protected:
  float* DepthMap;
  unsigned char* ColorMap;
  unsigned char* NormalMap;
  std::vector<float*> ScalarMaps;
  CameraCore Camera;
  double Domain[6];
  int Size[2];
  std::string FileName;

private:
  Frame(const Frame&); // Not implemented.
  void operator=(const Frame&); // Not implemented.
};

#endif
