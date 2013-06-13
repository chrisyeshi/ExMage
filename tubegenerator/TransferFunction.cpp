#include "TransferFunction.h"

#include <iostream>

TransferFunction::TransferFunction()
{
}

TransferFunction::~TransferFunction()
{
}

void TransferFunction::SetRGBAPoint(const float& value, const Color& color)
{
  Color cc = color;
  cc.r = color.r / 255.0;
  cc.g = color.g / 255.0;
  cc.b = color.b / 255.0;
//  cc.a = color.a / 255.0;
  Ctrls[value] = cc;
}

Color TransferFunction::GetColor(const float& value)
{
  if (Ctrls.empty())
    return Color();

  std::map<float, Color>::iterator itr[2];
  itr[0] = Ctrls.lower_bound(value);
  if (Ctrls.end() == itr[0])
  {
    --itr[0];
    return itr[0]->second;
  }
  itr[1] = itr[0];
  --itr[0];
  Color key_colors[2];
  key_colors[0] = itr[0]->second;
  key_colors[1] = itr[1]->second;
  float ratio = (value - itr[0]->first) / (itr[1]->first - itr[0]->first);
  Color ret;
  ret.r = key_colors[0].r * (1 - ratio) + key_colors[1].r * ratio;
  ret.g = key_colors[0].g * (1 - ratio) + key_colors[1].g * ratio;
  ret.b = key_colors[0].b * (1 - ratio) + key_colors[1].b * ratio;
  ret.a = key_colors[0].a * (1 - ratio) + key_colors[1].a * ratio;
  return ret;
}

void TransferFunction::CreateTexture(const int resolution)
{
  glGenTextures(1, &Tex);
  glBindTexture(GL_TEXTURE_1D, Tex);
  unsigned char* data = new unsigned char [resolution * 4];
  for (int i = 0; i < resolution; ++i)
  {
    Color color = this->GetColor(1.0 / float(resolution - 1) * float(i));
    data[4 * i + 0] = color.r * 255.0;
    data[4 * i + 1] = color.g * 255.0;
    data[4 * i + 2] = color.b * 255.0;
    data[4 * i + 3] = color.a * 255.0;
  }
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_1D, 0);
  delete [] data;
}

void TransferFunction::DeleteTexture()
{
  glDeleteTextures(1, &Tex);
}
