#ifndef __PNGWriter_h
#define __PNGWriter_h

#include <string>
#include <png.h>

class PNGWriter
{
public:
  PNGWriter() {};
  PNGWriter(const std::string& filename, int width, int height);
  ~PNGWriter() {};

  void SetFileName(const std::string& filename) {FileName = filename;};
  void SetSize(int width, int height) {Size[0] = width; Size[1] = height;};
  void SetSize(int size[2]) {Size[0] = size[0]; Size[1] = size[1];};

  bool Write(const unsigned char* buffer);
  bool Write(const float* buffer);

protected:
  std::string FileName;
  int Size[2]; // width, height

  FILE* fp;
  png_structp png_ptr;
  png_infop info_ptr;

  bool write_init();
  void write_header();
  void write_bytes(const unsigned char* buffer);
  bool write_end();
};

#endif
