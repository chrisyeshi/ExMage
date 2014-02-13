#ifndef __APNGWRITER_H_
#define __APGNWRITER_H_

#include <vector>
#include <string>

#include "zlib.h"

#include "CameraCore.h"

class Frame;

typedef struct { z_stream zstream; unsigned char* zbuf; int x, y, w, h, valid; } OP;
typedef struct { unsigned int num; unsigned char r, g, b, a; } COLORS;
typedef struct { unsigned char r, g, b; } rgb;
typedef struct
{
  unsigned char* p;
  unsigned int w, h;
  int t, ps, ts; rgb pl[256];
  unsigned char tr[256];
  unsigned short num, dem;
  unsigned int bpp;
} image_info;

class APNGWriter
{
public:
  APNGWriter();
  ~APNGWriter();

  void setFileName(const std::string& name) {filename = name;};
  void setSize(int ssize[2]);
  void setSize(int width, int height);
  void setGlobalDomain(double domain[6]);
  void setNumberOfScalarMaps(unsigned int mScalarMap) {this->mScalarMap = mScalarMap;};
  void setCamera(const CameraCore& cam) {camera = cam;};
  void setCompressionMode(int mode) {compression_mode = mode;};
  void setIdEnabled(bool idEnable) {id_enabled = idEnable;};
  void addImage(unsigned char* buffer);
  void addImage(unsigned short* buffer);
  void addImage(unsigned int* buffer);
  void addImage(float* buffer);
  void write();

protected:
  void write_chunk(FILE* f, const char* name, unsigned char* data, unsigned int length);
  void write_IDATs(FILE* f, int frame, unsigned char* data, unsigned int length, unsigned int idat_size);
  unsigned int get_rect(unsigned int w, unsigned h, unsigned char* pimg1, unsigned char* pimg2, unsigned char* ptemp, unsigned int* px, unsigned int* py, unsigned int* pw, unsigned int* ph, unsigned int bpp, unsigned int has_tcolor, unsigned int tcolor);
  void deflate_rect(unsigned char* pdata, int x, int y, int w, int h, int bpp, int stride, int zbuf_size, int n);

private:
  std::string filename;
  std::vector<image_info> images;
  int size[2];
  float global_domain[6];
  unsigned int mScalarMap;
  CameraCore camera;
  int compression_mode;
  bool id_enabled;
  OP op[12];
  COLORS col[256];
  unsigned int next_seq_num;
  unsigned char* row_buf;
  unsigned char* sub_row;
  unsigned char* up_row;
  unsigned char* avg_row;
  unsigned char* paeth_row;

  unsigned int rowbytes(unsigned int bpp) const {return size[0] * bpp;};
  unsigned int imagesize(unsigned int bpp) const {return rowbytes(bpp) * size[1];};
  unsigned int idat_size(unsigned int bpp) const {return (rowbytes(bpp) + 1) * size[1];};
  unsigned int zbuf_size(unsigned int bpp) const {return idat_size(bpp) + ((idat_size(bpp) + 7) >> 3) + ((idat_size(bpp) + 63) >> 6) + 11;};
};

#endif
