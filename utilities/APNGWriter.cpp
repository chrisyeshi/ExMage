#include "APNGWriter.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Frame.h"

#if defined(_MSC_VER) && _MSC_VER >= 1300
#define swap16(data) _byteswap_ushort(data)
#define swap32(data) _byteswap_ulong(data)
#elif defined(__linux__)
#include <byteswap.h>
#define swap16(data) bswap_16(data)
#define swap32(data) bswap_32(data)
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#define swap16(data) bswap16(data)
#define swap32(data) bswap32(data)
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define swap16(data) OSSwapInt16(data)
#define swap32(data) OSSwapInt32(data)
#else
unsigned short swap16(unsigned short data) {return((data & 0xFF) << 8) | ((data >> 8) & 0xFF);}
unsigned int swap32(unsigned int data) {return((data & 0xFF) << 24) | ((data & 0xFF00) << 8) | ((data >> 8) & 0xFF00) | ((data >> 24) & 0xFF);}
#endif

APNGWriter::APNGWriter() : next_seq_num(0)
{
}

APNGWriter::~APNGWriter()
{
  for (unsigned int i = 0; i < images.size(); ++i)
  {
    delete [] images[i].p;
  }
}

void APNGWriter::setSize(int ssize[2])
{
  size[0] = ssize[0];
  size[1] = ssize[1];
}

void APNGWriter::setSize(int width, int height)
{
  size[0] = width;
  size[1] = height;
}

void APNGWriter::setGlobalDomain(double domain[6])
{
  for (int i = 0; i < 6; ++i)
    global_domain[i] = domain[i];
}

void APNGWriter::addImage(unsigned char* buffer)
{
  image_info img;
  img.p = buffer;
  img.w = size[0];
  img.h = size[1];
  img.t = 0;
  img.ps = 0;
  img.ts = 0;
  rgb forpl;
  forpl.r = 0;
  forpl.g = 0;
  forpl.b = 0;
  for (int i = 0; i < 256; ++i)
  {
    img.pl[i] = forpl;
    img.tr[i] = 0;
  }
  img.num = 0;
  img.dem = 0;
  images.push_back(img);
}

void APNGWriter::addImage(unsigned short* buffer)
{
  addImage(reinterpret_cast<unsigned char *>(buffer));
}

void APNGWriter::addImage(unsigned int* buffer)
{
  addImage(reinterpret_cast<unsigned char *>(buffer));
}

void APNGWriter::addImage(float* buffer)
{
  addImage(reinterpret_cast<unsigned char *>(buffer));
}

void APNGWriter::write()
{
  char           * szOut;
  char           * szImage;
  char           * szOption;
  char             szFormat[256];
  char             szNext[256];
  unsigned int     i, j, k;
  unsigned int     width, height, len;
  unsigned int     x0, y0, w0, h0;
  unsigned int     x1, y1, w1, h1, try_over;
  unsigned int     bpp, rowbytes, imagesize, coltype;
  unsigned int     idat_size, zbuf_size, zsize;
  unsigned int     has_tcolor, tcolor, colors;
  unsigned int     frame_count, loops, cur, first;
  unsigned char    dop, bop, r, g, b, a;
  int              c;
  rgb              palette[256];
  unsigned char    trns[256];
  unsigned int     palsize, trnssize;
  unsigned char    cube[4096];
  unsigned char    gray[256];
  unsigned char  * zbuf;
  unsigned char  * sp;
  unsigned char  * dp;
  FILE           * f;
  unsigned char  * img_temp;
  image_info     * img;
  char           * szExt;
  unsigned char  * dst;
  short   delay_num = -1;
  short   delay_den = -1;
  int     input_ext = 0;
  int     keep_palette = 0;
  int     keep_coltype = 0;

  // initialize variables
  coltype = 6;
  bpp = 4;
  width = size[0];
  height = size[1];
  first = 0;
  frame_count = images.size();
  loops = 0;
  rowbytes = width * bpp;
  imagesize = rowbytes * height;
  idat_size = (rowbytes + 1) * height;
  zbuf_size = idat_size + ((idat_size + 7) >> 3) + ((idat_size + 63) >> 6) + 11;
  has_tcolor = 1;
  tcolor = 0;

  // deflateInit and init op
  for (i = 0; i < 12; ++i)
  {
    op[i].zstream.data_type = Z_BINARY;
    op[i].zstream.zalloc = Z_NULL;
    op[i].zstream.zfree = Z_NULL;
    op[i].zstream.opaque = Z_NULL;

    if (i & 1)
      deflateInit2(&op[i].zstream, Z_BEST_COMPRESSION, 8, 15, 8, Z_FILTERED);
    else
      deflateInit2(&op[i].zstream, Z_BEST_COMPRESSION, 8, 15, 8, Z_DEFAULT_STRATEGY);
  }

  // allocate memory
  for (i = 0; i < 12; ++i)
  {
    op[i].zbuf = (unsigned char*)malloc(zbuf_size);
    if (op[i].zbuf == NULL)
    {
      std::cout << "Error: not enougth memory" << std::endl;
      return;
    }
  }
  img_temp = (unsigned char*) malloc(imagesize);
  zbuf = (unsigned char*) malloc(zbuf_size);
  row_buf = (unsigned char*) malloc(rowbytes + 1);
  sub_row = (unsigned char*) malloc(rowbytes + 1);
  up_row = (unsigned char*) malloc(rowbytes + 1);
  avg_row = (unsigned char*) malloc(rowbytes + 1);
  paeth_row = (unsigned char*) malloc(rowbytes + 1);

  if (img_temp && zbuf && row_buf && sub_row && up_row && avg_row && paeth_row)
  {
    row_buf[0] = 0;
    sub_row[0] = 1;
    up_row[0] = 2;
    avg_row[0] = 3;
    paeth_row[0] = 4;
  }
  else
  {
    std::cout << "Error: not enough memory" << std::endl;
    return;
  }

  // struct IHDR
  struct IHDR
  {
    unsigned int	mWidth;
    unsigned int	mHeight;
    unsigned char	mDepth;
    unsigned char	mColorType;
    unsigned char	mCompression;
    unsigned char	mFilterMethod;
    unsigned char	mInterlaceMethod;
  } ihdr = { swap32(width), swap32(height), 8, coltype, 0, 0, 0 };

  // struct gdEI
  struct gdEI
  {
    unsigned int	mFrameCount;
    int			globalDomain[6];
    unsigned int	mScalarMap;
    int			camera[9];
    unsigned char	compressionMode;
    unsigned char	idEnabled;
  } gdei;
  gdei.mFrameCount = swap32(frame_count - first);
  for (int i = 0; i < 6; ++i)
  {
    float tmp = global_domain[i];
    int swap = 0;
    memcpy(&swap, &tmp, sizeof(tmp));
    gdei.globalDomain[i] = swap32(swap);
  }
  gdei.mScalarMap = swap32(mScalarMap);
  float ftemp = 0.0;
  int itemp = 0;
  ftemp = camera.Position.x();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[0] = swap32(itemp);
  ftemp = camera.Position.y();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[1] = swap32(itemp);
  ftemp = camera.Position.z();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[2] = swap32(itemp);
  ftemp = camera.Focal.x();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[3] = swap32(itemp);
  ftemp = camera.Focal.y();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[4] = swap32(itemp);
  ftemp = camera.Focal.z();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[5] = swap32(itemp);
  ftemp = camera.ViewUp.x();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[6] = swap32(itemp);
  ftemp = camera.ViewUp.y();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[7] = swap32(itemp);
  ftemp = camera.ViewUp.z();
  memcpy(&itemp, &ftemp, sizeof(ftemp));
  gdei.camera[8] = swap32(itemp);
  gdei.compressionMode = compression_mode;
  gdei.idEnabled = (id_enabled ? 1 : 0);

  // struct ldEI
  struct ldEI
  {
    unsigned int	mSeq;
    unsigned int	mWidth;
    unsigned int	mHeight;
    unsigned int	mXOffset;
    unsigned int	mYOffset;
    unsigned char	mDisposeOp;
    unsigned char	mBlendOp;
  } ldei;

  // open output file
  f = fopen(filename.c_str(), "wb");
  if (!f)
    return;

  // png signature
  unsigned char png_sign[8] = {137, 80, 78, 71, 13, 10, 26, 10};
  fwrite(png_sign, 1, 8, f);

  // IHDR chunk
  write_chunk(f, "IHDR", (unsigned char*)(&ihdr), 13);

  // gdEI chunk
  write_chunk(f, "gdEI", (unsigned char*)(&gdei), 70);

  x0 = 0;
  y0 = 0;
  w0 = width;
  h0 = height;
  bop = 0;

  printf("saving %s (frame %d of %d)\n", filename.c_str(), 1 - first, frame_count - first);
  deflate_rect(images[0].p, x0, y0, w0, h0, bpp, rowbytes, zbuf_size, 0);

  if (op[0].zstream.total_out <= op[1].zstream.total_out)
  {
    zsize = op[0].zstream.total_out;
    memcpy(zbuf, op[0].zbuf, zsize);
  }
  else
  {
    zsize = op[1].zstream.total_out;
    memcpy(zbuf, op[1].zbuf, zsize);
  }

  deflateReset(&op[0].zstream);
  op[0].zstream.data_type = Z_BINARY;
  deflateReset(&op[1].zstream);
  op[1].zstream.data_type = Z_BINARY;

  for (i = first; i < frame_count - 1; ++i)
  {
    unsigned int	op_min;
    int			op_best;

    printf("saving %s (frame %d of %d)\n", filename.c_str(), i - first + 2, frame_count - first);
    for (j = 0; j < 12; ++j)
      op[j].valid = 0;

    /* dispose = none */
    try_over = get_rect(width, height, images[i].p, images[i + 1].p, img_temp, &x1, &y1, &w1, &h1, bpp, has_tcolor, tcolor);
    deflate_rect(images[i + 1].p, x1, y1, w1, h1, bpp, rowbytes, zbuf_size, 0);
    if (try_over)
      deflate_rect(img_temp, x1, y1, w1, h1, bpp, rowbytes, zbuf_size, 1);

    /* dispose = background */
    if (has_tcolor)
    {
      memcpy(img_temp, images[i].p, imagesize);
      if (coltype == 2)
        for (j = 0; j < h0; ++j)
          for (k = 0; k < w0; ++k)
            memcpy(img_temp + ((j + y0) * width + (k + x0)) * 3, &tcolor, 3);
      else
        for (j = 0; j < h0; ++j)
          memset(img_temp + ((j + y0) * width + x0) * bpp, tcolor, w0 * bpp);

      try_over = get_rect(width, height, img_temp, images[i + 1].p, img_temp, &x1, &y1, &w1, &h1, bpp, has_tcolor, tcolor);

      deflate_rect(images[i + 1].p, x1, y1, w1, h1, bpp, rowbytes, zbuf_size, 2);
      if (try_over)
        deflate_rect(img_temp, x1, y1, w1, h1, bpp, rowbytes, zbuf_size, 3);
    }

    if (i > first)
    {
      /* dispose = previous */
      try_over = get_rect(width, height, images[i - 1].p, images[i + 1].p, img_temp, &x1, &y1, &w1, &h1, bpp, has_tcolor, tcolor);
      deflate_rect(images[i + 1].p, x1, y1, w1, h1, bpp, rowbytes, zbuf_size, 4);
      if (try_over)
        deflate_rect(img_temp, x1, y1, w1, h1, bpp, rowbytes, zbuf_size, 5);
    }

    op_min = op[0].zstream.total_out;
    op_best = 0;
    for (j = 1; j < 12; ++j)
    {
      if (op[j].valid)
      {
        if (op[j].zstream.total_out < op_min)
        {
          op_min = op[j].zstream.total_out;
          op_best = j;
        }
      }
    }

    dop = op_best >> 2;

    ldei.mSeq		= swap32(next_seq_num++);
    ldei.mWidth		= swap32(w0);
    ldei.mHeight	= swap32(h0);
    ldei.mXOffset	= swap32(x0);
    ldei.mYOffset	= swap32(y0);
    ldei.mDisposeOp	= dop;
    ldei.mBlendOp	= bop;
    write_chunk(f, "ldEI", (unsigned char*)(&ldei), 22);

    write_IDATs(f, i, zbuf, zsize, idat_size);

    /* process apng dispose - begin */
    if (dop == 1)
      for (j = 0; j < h0; ++j)
        memset(images[i].p + ((j + y0) * width + x0) * bpp, tcolor, w0 * bpp);
    else
    if (dop == 2)
      for (j = 0; j < h0; ++j)
        memcpy(images[i].p + ((j + y0) * width + x0) * bpp, images[i - 1].p + ((j + y0) * width + x0) * bpp, w0 * bpp);
    /* process apng dispose - end */

    x0 = op[op_best].x;
    y0 = op[op_best].y;
    w0 = op[op_best].w;
    h0 = op[op_best].h;
    bop = (op_best >> 1) & 1;

    zsize = op[op_best].zstream.total_out;
    memcpy(zbuf, op[op_best].zbuf, zsize);

    for (j = 0; j < 12; ++j)
    {
      deflateReset(&op[j].zstream);
      op[j].zstream.data_type = Z_BINARY;
    }
  }

  if (frame_count > 1)
  {
    ldei.mSeq		= swap32(next_seq_num++);
    ldei.mWidth		= swap32(w0);
    ldei.mHeight	= swap32(h0);
    ldei.mXOffset	= swap32(x0);
    ldei.mYOffset	= swap32(y0);
    ldei.mDisposeOp	= 0;
    ldei.mBlendOp	= bop;
    write_chunk(f, "ldEI", (unsigned char*)(&ldei), 22);
  }

  write_IDATs(f, i, zbuf, zsize, idat_size);

  unsigned char png_Software[27] = { 83, 111, 102, 116, 119, 97, 114, 101, '\0', 
                                     65,  80,  78,  71,  32, 65, 115, 115, 101, 
                                    109,  98, 108, 101, 114, 32,  50,  46,  55};
  write_chunk(f, "tEXt", png_Software, 27);
  write_chunk(f, "IEND", 0, 0);

  // close the file
  fclose(f);

  for (i = 0; i < 12; ++i)
  {
    deflateEnd(&op[i].zstream);
    if (op[i].zbuf != NULL)
      free(op[i].zbuf);
  }

  free(img_temp);
  free(zbuf);
  free(row_buf);
  free(sub_row);
  free(up_row);
  free(avg_row);
  free(paeth_row);
}

void APNGWriter::write_chunk(FILE * f, const char * name, unsigned char * data, unsigned int length)
{
  unsigned int crc = crc32(0, Z_NULL, 0);
  unsigned int len = swap32(length);

  fwrite(&len, 1, 4, f);
  fwrite(name, 1, 4, f);
  crc = crc32(crc, (const Bytef *)name, 4);

  if (memcmp(name, "fdAT", 4) == 0)
  {
    unsigned int seq = swap32(next_seq_num++);
    fwrite(&seq, 1, 4, f);
    crc = crc32(crc, (const Bytef *)(&seq), 4);
    length -= 4;
  }

  if (data != NULL && length > 0)
  {
    fwrite(data, 1, length, f);
    crc = crc32(crc, data, length);
  }

  crc = swap32(crc);
  fwrite(&crc, 1, 4, f);
}

void APNGWriter::write_IDATs(FILE * f, int frame, unsigned char * data, unsigned int length, unsigned int idat_size)
{
  unsigned int z_cmf = data[0];
  if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
  {
    if (length >= 2)
    {
      unsigned int z_cinfo = z_cmf >> 4;
      unsigned int half_z_window_size = 1 << (z_cinfo + 7);
      while (idat_size <= half_z_window_size && half_z_window_size >= 256)
      {
        z_cinfo--;
        half_z_window_size >>= 1;
      }
      z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);
      if (data[0] != (unsigned char)z_cmf)
      {
        data[0] = (unsigned char)z_cmf;
        data[1] &= 0xe0;
        data[1] += (unsigned char)(0x1f - ((z_cmf << 8) + data[1]) % 0x1f);
      }
    }
  }

  while (length > 0)
  {
    unsigned int ds = length;
    if (ds > 32768)
      ds = 32768;

    if (frame == 0)
      write_chunk(f, "IDAT", data, ds);
    else
      write_chunk(f, "fdAT", data, ds+4);

    data += ds;
    length -= ds;
  }
}

unsigned int APNGWriter::get_rect(unsigned int w, unsigned int h, unsigned char *pimg1, unsigned char *pimg2, unsigned char *ptemp, unsigned int *px, unsigned int *py, unsigned int *pw, unsigned int *ph, unsigned int bpp, unsigned int has_tcolor, unsigned int tcolor)
{
  unsigned int   i, j;
  unsigned int   x_min = w-1;
  unsigned int   y_min = h-1;
  unsigned int   x_max = 0;
  unsigned int   y_max = 0;
  unsigned int   diffnum = 0;
  unsigned int   over_is_possible = 1;

  if (!has_tcolor)
    over_is_possible = 0;

  if (bpp == 1)
  {
    unsigned char *pa = pimg1;
    unsigned char *pb = pimg2;
    unsigned char *pc = ptemp;

    for (j=0; j<h; j++)
    for (i=0; i<w; i++)
    {
      unsigned char c = *pb++;
      if (*pa++ != c)
      {
        diffnum++;
        if ((has_tcolor) && (c == tcolor)) over_is_possible = 0;
        if (i<x_min) x_min = i;
        if (i>x_max) x_max = i;
        if (j<y_min) y_min = j;
        if (j>y_max) y_max = j;
      }
      else
        c = tcolor;

      *pc++ = c;
    }
  }
  else
  if (bpp == 2)
  {
    unsigned short *pa = (unsigned short *)pimg1;
    unsigned short *pb = (unsigned short *)pimg2;
    unsigned short *pc = (unsigned short *)ptemp;

    for (j=0; j<h; j++)
    for (i=0; i<w; i++)
    {
      unsigned int c1 = *pa++;
      unsigned int c2 = *pb++;
      if ((c1 != c2) && ((c1>>8) || (c2>>8)))
      {
        diffnum++;
        if ((c2 >> 8) != 0xFF) over_is_possible = 0;
        if (i<x_min) x_min = i;
        if (i>x_max) x_max = i;
        if (j<y_min) y_min = j;
        if (j>y_max) y_max = j;
      }
      else
        c2 = 0;

      *pc++ = c2;
    }
  }
  else
  if (bpp == 3)
  {
    unsigned char *pa = pimg1;
    unsigned char *pb = pimg2;
    unsigned char *pc = ptemp;

    for (j=0; j<h; j++)
    for (i=0; i<w; i++)
    {
      unsigned int c1 = (((pa[2]<<8)+pa[1])<<8)+pa[0];
      unsigned int c2 = (((pb[2]<<8)+pb[1])<<8)+pb[0];
      if (c1 != c2)
      {
        diffnum++;
        if ((has_tcolor) && (c2 == tcolor)) over_is_possible = 0;
        if (i<x_min) x_min = i;
        if (i>x_max) x_max = i;
        if (j<y_min) y_min = j;
        if (j>y_max) y_max = j;
      }
      else
        c2 = tcolor;

      memcpy(pc, &c2, 3);
      pa += 3;
      pb += 3;
      pc += 3;
    }
  }
  else
  if (bpp == 4)
  {
    unsigned int *pa = (unsigned int *)pimg1;
    unsigned int *pb = (unsigned int *)pimg2;
    unsigned int *pc = (unsigned int *)ptemp;

    for (j=0; j<h; j++)
    for (i=0; i<w; i++)
    {
      unsigned int c1 = *pa++;
      unsigned int c2 = *pb++;
      if ((c1 != c2) && ((c1>>24) || (c2>>24)))
      {
        diffnum++;
        if ((c2 >> 24) != 0xFF) over_is_possible = 0;
        if (i<x_min) x_min = i;
        if (i>x_max) x_max = i;
        if (j<y_min) y_min = j;
        if (j>y_max) y_max = j;
      }
      else
        c2 = 0;

      *pc++ = c2;
    }
  }

  if (diffnum == 0)
  {
    *px = *py = 0;
    *pw = *ph = 1; 
  }
  else
  {
    *px = x_min;
    *py = y_min;
    *pw = x_max-x_min+1;
    *ph = y_max-y_min+1;
  }

  return over_is_possible;
}

void APNGWriter::deflate_rect(unsigned char *pdata, int x, int y, int w, int h, int bpp, int stride, int zbuf_size, int n)
{
  int i, j, v;
  int a, b, c, pa, pb, pc, p;
  int rowbytes = w * bpp;
  unsigned char * prev = NULL;
  unsigned char * row  = pdata + y*stride + x*bpp;
  unsigned char * out;

  op[n*2].valid = 1;
  op[n*2].zstream.next_out = op[n*2].zbuf;
  op[n*2].zstream.avail_out = zbuf_size;

  op[n*2+1].valid = 1;
  op[n*2+1].zstream.next_out = op[n*2+1].zbuf;
  op[n*2+1].zstream.avail_out = zbuf_size;

  for (j=0; j<h; j++)
  {
    unsigned int    sum = 0;
    unsigned char * best_row = row_buf;
    unsigned int    mins = ((unsigned int)(-1)) >> 1;

    out = row_buf+1;
    for (i=0; i<rowbytes; i++)
    {
      v = out[i] = row[i];
      sum += (v < 128) ? v : 256 - v;
    }
    mins = sum;

    sum = 0;
    out = sub_row+1;
    for (i=0; i<bpp; i++)
    {
      v = out[i] = row[i];
      sum += (v < 128) ? v : 256 - v;
    }
    for (i=bpp; i<rowbytes; i++)
    {
      v = out[i] = row[i] - row[i-bpp];
      sum += (v < 128) ? v : 256 - v;
      if (sum > mins) break;
    }
    if (sum < mins)
    {
      mins = sum;
      best_row = sub_row;
    }

    if (prev)
    {
      sum = 0;
      out = up_row+1;
      for (i=0; i<rowbytes; i++)
      {
        v = out[i] = row[i] - prev[i];
        sum += (v < 128) ? v : 256 - v;
        if (sum > mins) break;
      }
      if (sum < mins)
      {
        mins = sum;
        best_row = up_row;
      }

      sum = 0;
      out = avg_row+1;
      for (i=0; i<bpp; i++)
      {
        v = out[i] = row[i] - prev[i]/2;
        sum += (v < 128) ? v : 256 - v;
      }
      for (i=bpp; i<rowbytes; i++)
      {
        v = out[i] = row[i] - (prev[i] + row[i-bpp])/2;
        sum += (v < 128) ? v : 256 - v;
        if (sum > mins) break;
      }
      if (sum < mins)
      { 
        mins = sum;
        best_row = avg_row;
      }
      sum = 0;
      out = paeth_row+1;
      for (i=0; i<bpp; i++)
      {
        v = out[i] = row[i] - prev[i];
        sum += (v < 128) ? v : 256 - v;
      }
      for (i=bpp; i<rowbytes; i++)
      {
        a = row[i-bpp];
        b = prev[i];
        c = prev[i-bpp];
        p = b - c;
        pc = a - c;
        pa = abs(p);
        pb = abs(pc);
        pc = abs(p + pc);
        p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;
        v = out[i] = row[i] - p;
        sum += (v < 128) ? v : 256 - v;
        if (sum > mins) break;
      }
      if (sum < mins)
      {
        best_row = paeth_row;
      }
    }

    op[n*2].zstream.next_in = row_buf;
    op[n*2].zstream.avail_in = rowbytes + 1;
    deflate(&op[n*2].zstream, Z_NO_FLUSH);

    op[n*2+1].zstream.next_in = best_row;
    op[n*2+1].zstream.avail_in = rowbytes + 1;
    deflate(&op[n*2+1].zstream, Z_NO_FLUSH);

    prev = row;
    row += stride;
  }

  deflate(&op[n*2].zstream, Z_FINISH);
  deflate(&op[n*2+1].zstream, Z_FINISH);

  op[n*2].x = op[n*2+1].x = x;
  op[n*2].y = op[n*2+1].y = y;
  op[n*2].w = op[n*2+1].w = w;
  op[n*2].h = op[n*2+1].h = h;
}
