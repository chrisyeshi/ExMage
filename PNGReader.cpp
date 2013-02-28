#include "PNGReader.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <setjmp.h>

bool PNGReader::Read(unsigned char** buffer)
{
  if (!read())
    return false;
  // copy to buffer
  *buffer = new unsigned char [Size[0] * Size[1] * 4];
  for (int y = 0; y < Size[1]; ++y)
    memcpy(&(*buffer)[4 * Size[0] * (Size[1] - 1 - y)], row_pointers[y], 4 * Size[0]);
  return true;
}

bool PNGReader::Read(float** buffer)
{
  if (!read())
    return false;
  // copy to buffer
  *buffer = new float [Size[0] * Size[1]];
  for (int y = 0; y < Size[1]; ++y)
    memcpy(&(*buffer)[Size[0] * (Size[1] - 1 - y)], row_pointers[y], 4 * Size[0]);
  return true;
}

bool PNGReader::read()
{
  png_byte header[8];
  // open file and test for it being a png
  FILE* fp = fopen(FileName.c_str(), "rb");
  if (!fp)
    return false;
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8))
    return false;
  // initialize stuff
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return false;
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    return false;
  if (setjmp(png_jmpbuf(png_ptr)))
    return false;

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  Size[0] = png_get_image_width(png_ptr, info_ptr);
  Size[1] = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  // read file
  if (setjmp(png_jmpbuf(png_ptr)))
    return false;
  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * Size[1]);
  for (int y = 0; y < Size[1]; ++y)
    row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr, info_ptr));

  png_read_image(png_ptr, row_pointers);

  fclose(fp);
  return true;
}
