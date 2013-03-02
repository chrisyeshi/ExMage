#include "Frame.h"

#include <assert.h>
#include <string>
#include <fstream>
#include <iostream>

#include "PNGWriter.h"
#include "PNGReader.h"

/////////////////////////////////////////////////////////////////////////////////////
//
// class Frame
//
/////////////////////////////////////////////////////////////////////////////////////

Frame::Frame() : DepthMap(NULL), ColorMap(NULL), NormalMap(NULL)
{
  for (int i = 0; i < 6; ++i)
    Domain[i] = 0.0;
  Size[0] = Size[1] = 0;
}

Frame::~Frame()
{
  if (DepthMap)
    delete [] DepthMap;
  if (ColorMap)
    delete [] ColorMap;
  if (NormalMap)
    delete [] NormalMap;
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    if (ScalarMaps[i])
      delete [] ScalarMaps[i];
  }
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////////////////

void Frame::Reset()
{
  if (DepthMap)
  {
    delete [] DepthMap;
    DepthMap = NULL;
  }
  if (ColorMap)
  {
    delete [] ColorMap;
    ColorMap = NULL;
  }
  if (NormalMap)
  {
    delete [] NormalMap;
    NormalMap = NULL;
  }
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
    delete [] ScalarMaps[i];
  ScalarMaps.clear();
}

float* Frame::GetScalarMap(const int index) const
{
  if (index < 0 || index >= int(ScalarMaps.size()))
    return NULL;
  return ScalarMaps[index];
}

void Frame::SetDepthMap(const float* depth)
{
  if (DepthMap)
    delete [] DepthMap;
  DepthMap = new float [Size[0] * Size[1]];
  memcpy(DepthMap, depth, sizeof(float) * Size[0] * Size[1]);
}

void Frame::SetColorMap(const unsigned char* color)
{
  if (ColorMap)
    delete [] ColorMap;
  ColorMap = new unsigned char [Size[0] * Size[1] * 4];
  memcpy(ColorMap, color, sizeof(unsigned char) * Size[0] * Size[1] * 4);
}

void Frame::SetNormalMap(const unsigned char* normal)
{
  if (NormalMap)
    delete [] NormalMap;
  NormalMap = new unsigned char [Size[0] * Size[1] * 4];
  memcpy(NormalMap, normal, sizeof(unsigned char) * Size[0] * Size[1] * 4);
}

void Frame::SetNumberOfScalarMaps(const int num)
{
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    delete [] ScalarMaps[i];
  }
  ScalarMaps.resize(num);
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    ScalarMaps[i] = NULL;
  }
}

void Frame::SetScalarMap(const int index, const float* scalar)
{
  if (index < 0 || index >= int(ScalarMaps.size()))
  {
    return;
  }
  if (ScalarMaps[index])
    delete [] ScalarMaps[index];
  ScalarMaps[index] = new float [Size[0] * Size[1]];
  memcpy(ScalarMaps[index], scalar, sizeof(float) * Size[0] * Size[1]);
}

void Frame::SetSize(int width, int height)
{
  int size[2] = {width, height};
  this->SetSize(size);
}

void Frame::SetSize(int size[2])
{
  Size[0] = size[0];
  Size[1] = size[1];
  // delete the original data if exists
  if (DepthMap)
  {
    delete [] DepthMap;
    DepthMap = NULL;
  }
  if (ColorMap)
  {
    delete [] ColorMap;
    ColorMap = NULL;
  }
  if (NormalMap)
  {
    delete [] NormalMap;
    NormalMap = NULL;
  }
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    delete [] ScalarMaps[i];
    ScalarMaps[i] = NULL;
  }
}

void Frame::GetDataDomain(double domain[6]) const
{
  for (int i = 0; i < 6; ++i)
    domain[i] = Domain[i];
}

void Frame::SetDataDomain(double domain[6])
{
  for (int i = 0; i < 6; ++i)
    Domain[i] = domain[i];
}

bool Frame::Read()
{
  if (FileName.empty())
    return false;

  std::ifstream fin;
  std::string spt_file = FileName + ".spt";
  fin.open(spt_file.c_str(), std::ios::in | std::ios::binary);
  if (!fin.good())
    return false;

  if (DepthMap)
  {
    delete [] DepthMap;
    DepthMap = NULL;
  }
  if (ColorMap)
  {
    delete [] ColorMap;
    ColorMap = NULL;
  }
  if (NormalMap)
  {
    delete [] NormalMap;
    NormalMap = NULL;
  }
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    if (ScalarMaps[i])
    {
      delete [] ScalarMaps[i];
      ScalarMaps[i] = NULL;
    }
  }
  // read domain, size, depth buffer then color buffer
  double cam[9];
  fin.read(reinterpret_cast<char *>(cam), sizeof(double) * 9);
  Camera.Position = Point(cam[0], cam[1], cam[2]);
  Camera.Focal = Point(cam[3], cam[4], cam[5]);
  Camera.ViewUp = Vector(cam[6], cam[7], cam[8]);
  fin.read(reinterpret_cast<char *>(Domain), sizeof(double) * 6);
  fin.read(reinterpret_cast<char *>(&Size[0]), sizeof(int) * 2);
  int num_element = Size[0] * Size[1];
  fin.close();
  // depth
  PNGReader depth_reader(FileName + "_depth.png");
  if (!depth_reader.Read(&DepthMap))
    return false;
  depth_reader.GetSize(Size);
  // color
  PNGReader color_reader(FileName + "_color.png");
  if (!color_reader.Read(&ColorMap))
    return false;
  color_reader.GetSize(Size);
  // normal
  PNGReader normal_reader(FileName + "_normal.png");
  if (!normal_reader.Read(&NormalMap))
    return false;
//  normal_reader.GetSize(Size);
  // scalars
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    char istr[5];
    sprintf(istr, "%d", i);
    PNGReader scalar_reader(FileName + "_scalar_" + istr);
    if (!scalar_reader.Read(&ScalarMaps[i]))
      return false;
    scalar_reader.GetSize(Size);
  }
  return true;
}

bool Frame::Write()
{
  if (FileName.empty())
    return false;

  std::ofstream fout;
  std::string spt_file = FileName + ".spt";
  fout.open(spt_file.c_str(), std::ios::out | std::ios::binary);
  if (!fout.good())
    return false;

  // write domain, size
  double cam[] = {Camera.Position.x, Camera.Position.y, Camera.Position.z,
                  Camera.Focal.x, Camera.Focal.y, Camera.Focal.z,
                  Camera.ViewUp.x, Camera.ViewUp.y, Camera.ViewUp.z};
  fout.write(reinterpret_cast<char *>(cam), sizeof(double) * 9);
  fout.write(reinterpret_cast<char *>(Domain), sizeof(double) * 6);
  fout.write(reinterpret_cast<char *>(&Size[0]), sizeof(int) * 2);
  fout.close();
  // depth
  PNGWriter depth_writer(FileName + "_depth.png", Size[0], Size[1]);
  depth_writer.Write(DepthMap);
  // color
  PNGWriter color_writer(FileName + "_color.png", Size[0], Size[1]);
  color_writer.Write(ColorMap);
  // normal
  PNGWriter normal_writer(FileName + "_normal.png", Size[0], Size[1]);
  normal_writer.Write(NormalMap);
  // scalars
  for (unsigned int i = 0; i < ScalarMaps.size(); ++i)
  {
    char istr[5];
    sprintf(istr, "%d", i);
    PNGWriter scalar_writer(FileName + "_scalar_" + istr + ".png", Size[0], Size[1]);
    scalar_writer.Write(ScalarMaps[i]);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////
//
// Protected Methods
//
/////////////////////////////////////////////////////////////////////////////////////
/*
void Frame::Composite(const unsigned char* color, const float* depth, const int x, const int y)
{
  if (!this->rgba_char_data && !this->z_buffer_data)
  {
    this->Size[0] = x;
    this->Size[1] = y;
    this->rgba_char_data = new unsigned char [this->Size[0] * this->Size[1] * 4];
    memcpy(this->rgba_char_data, color,
      sizeof(unsigned char) * this->Size[0] * this->Size[1] * 4);
    this->z_buffer_data = new float [this->Size[0] * this->Size[1]];
    memcpy(this->z_buffer_data, depth,
      sizeof(float) * this->Size[0] * this->Size[1]);
    return;
  }
  assert(this->Size[0] == x && this->Size[1] == y);
  // composite
  for (int x = 0; x < this->Size[0]; ++x)
  {
    for (int y = 0; y < this->Size[1]; ++y)
    {
      int element_index = y * this->Size[0] + x;
      float z1 = depth[element_index];
      float z2 = this->z_buffer_data[element_index];
      if (z1 <= z2)
      {
        this->z_buffer_data[element_index] = z1;
        this->rgba_char_data[element_index * 4 + 0] = color[element_index * 4 + 0];
        this->rgba_char_data[element_index * 4 + 1] = color[element_index * 4 + 1];
        this->rgba_char_data[element_index * 4 + 2] = color[element_index * 4 + 2];
        this->rgba_char_data[element_index * 4 + 3] = color[element_index * 4 + 3];
      }
    }
  }
}
*/
