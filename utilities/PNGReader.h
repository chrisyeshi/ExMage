#ifndef __PNGReader_h
#define __PNGReader_h

#include <string>
#include <png.h>

class PNGReader
{
public:
  PNGReader() {};
  PNGReader(const std::string& filename) : FileName(filename) {};
  ~PNGReader() {};

  void SetFileName(const std::string& filename) {FileName = filename;};
  void GetSize(int size[2]) {size[0] = Size[0]; size[1] = Size[1];};

  bool Read(unsigned char** buffer);
  bool Read(float** buffer);

protected:
  std::string FileName;
  int Size[2];

  FILE* fp;
  png_structp png_ptr;
  png_infop info_ptr;
  png_byte color_type;
  png_byte bit_depth;
  int number_of_passes;
  png_bytep* row_pointers;

  bool read();
};

#endif
