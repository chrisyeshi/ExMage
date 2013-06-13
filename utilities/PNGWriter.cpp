#include "PNGWriter.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <setjmp.h>

PNGWriter::PNGWriter(const std::string& filename, int width, int height)
  : FileName(filename), png_ptr(NULL), info_ptr(NULL)
{
  Size[0] = width;
  Size[1] = height;
}

bool PNGWriter::Write(const unsigned char* buffer)
{
  if (!write_init())
    return false;

  write_header();
  write_bytes(buffer);

  if (!write_end())
    return false;
  return true;
}

bool PNGWriter::Write(const float* buffer)
{
  if (!write_init())
    return false;

  write_header();
  write_bytes(reinterpret_cast<const unsigned char*>(buffer));

  if (!write_end())
    return false;
  return true;
}

bool PNGWriter::write_init()
{
  fp = fopen(FileName.c_str(), "wb");
  if (!fp)
    return false;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return false;

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    return false;

  if (setjmp(png_jmpbuf(png_ptr)))
    return false;

  png_init_io(png_ptr, fp);

  if (setjmp(png_jmpbuf(png_ptr)))
    return false;

  return true;
}

void PNGWriter::write_header()
{
  png_set_IHDR(png_ptr, info_ptr, Size[0], Size[1],
               8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);
}

void PNGWriter::write_bytes(const unsigned char *buffer)
{
  png_bytep row = (png_bytep) malloc(4 * Size[0] * sizeof(png_byte));
  for (int y = 1; y <= Size[1]; ++y)
  {
    memcpy(row, &buffer[4 * (Size[1] - y) * Size[0]], 4 * Size[0]);
    png_write_row(png_ptr, row);
  }
  free(row);
}

bool PNGWriter::write_end()
{
  if (setjmp(png_jmpbuf(png_ptr)))
    return false;

  png_write_end(png_ptr, NULL);

  // cleanup heap allocation
  png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

  fclose(fp);
  return true;
}
