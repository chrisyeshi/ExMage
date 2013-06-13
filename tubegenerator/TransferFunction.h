#ifndef __TransferFunction_h
#define __TransferFunction_h

#include <GL/gl.h>
#include <map>

class Color
{
public:
  Color() {};
  Color(float R, float G, float B, float A)
    : r(R), g(G), b(B), a(A) {};
  float r, g, b, a;
};

class TransferFunction
{
public:
  TransferFunction();
  ~TransferFunction();

  void SetRGBAPoint(const float& value, const Color& color);
  Color GetColor(const float& value);
  void CreateTexture(const int resolution);
  GLuint GetTexture() const {return Tex;};
  void DeleteTexture();

protected:
  std::map<float, Color> Ctrls;
  GLuint Tex;
};

#endif
