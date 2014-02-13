#ifndef __APNGREADER_H_
#define __APNGREADER_H_

#include <string>
#include <vector>

#include <png.h>

#include "CameraCore.h"

class Frame;

struct image_info
{
  unsigned int w, h, d, t, ch, it, w0, h0, x0, y0, frames, plays, cur;
  png_color pl[256]; int ps; png_byte tr[256]; int ts; png_color_16 tc;
  unsigned short delay_num, delay_den; unsigned char dop, bop;
  z_stream zstream;
  unsigned char * buf, * image, * frame, * restore; 
  unsigned int buf_size, size;
  png_bytepp rows_image;
  png_bytepp rows_frame;
};

class APNGReader
{
public:
  APNGReader();
  ~APNGReader();

  std::vector<int> getResolution() const;
  std::vector<float> getGlobalDomain() const;
  int getNumScalarMaps() const { return number_scalar_maps; }
  int getCompressionMode() const { return compression_mode; }
  bool getIdEnabled() const { return id_enabled; }
  CameraCore getCamera() const { return camera; }
  bool read();
  void setFileName(const std::string& name) { filename = name; }
  std::vector<Frame*> getFrames() const { return frames; }

protected:

private:
  std::string filename;
  std::vector<Frame*> frames;
  image_info img;
  int image_index;
  float global_domain[6];
  unsigned int number_scalar_maps;
  int compression_mode;
  bool id_enabled;
  CameraCore camera;

  static int handle_apng_chunks(png_struct* png_ptr, png_unknown_chunkp chunk);
  void SavePNG();
  void divDepthNormal(image_info* pimg, float* depthMap, float* normalMap);
  void convertNormal(image_info* pimg, float* normalMap);
};

#endif
