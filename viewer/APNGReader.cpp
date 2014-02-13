#include "APNGReader.h"

#include <cstdlib>
#include <cassert>
#include <climits>

#include "PNGWriter.h"
#include "Frame.h"

APNGReader::APNGReader()
{
}

APNGReader::~APNGReader()
{
  if (!frames.empty())
  {
    for (unsigned int i = 0; i < frames.size(); ++i)
      delete frames[i];
  }
}

std::vector<int> APNGReader::getResolution() const
{
    std::vector<int> ret(2);
    ret[0] = img.w;
    ret[1] = img.h;
    return ret;
}

std::vector<float> APNGReader::getGlobalDomain() const
{
    std::vector<float> domain(6);
    for (int i = 0; i < 6; ++i)
        domain[i] = global_domain[i];
    return domain;
}

bool APNGReader::read()
{
  if (filename.empty())
    return false;

  image_index = 0;
  FILE * f1;
  png_byte apng_chunks[]= {"gdEI\0ldEI\0fdAT\0"};

  if ((f1 = fopen(filename.c_str(), "rb")) != 0)
  {
    png_colorp      palette;
    png_color_16p   trans_color;
    png_bytep       trans_alpha;
    unsigned int    rowbytes, j;
    unsigned char   sig[8];
//    image_info      img;

    memset(&img, 0, sizeof(img));
    memset(img.tr, 255, 256);
    img.zstream.zalloc  = Z_NULL;
    img.zstream.zfree   = Z_NULL;
    img.zstream.opaque  = Z_NULL;
    inflateInit(&img.zstream);

    if (fread(sig, 1, 8, f1) == 8 && png_sig_cmp(sig, 0, 8) == 0)
    {
      png_structp png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      png_infop   info_ptr = png_create_info_struct(png_ptr);
      if (png_ptr != NULL && info_ptr != NULL && setjmp(png_jmpbuf(png_ptr)) == 0)
      {
        png_set_keep_unknown_chunks(png_ptr, 2, apng_chunks, 3);
        png_set_read_user_chunk_fn(png_ptr, this, APNGReader::handle_apng_chunks);
        png_init_io(png_ptr, f1);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        img.w    = png_get_image_width(png_ptr, info_ptr);
        img.h    = png_get_image_height(png_ptr, info_ptr);
        img.d    = png_get_bit_depth(png_ptr, info_ptr);
        img.t    = png_get_color_type(png_ptr, info_ptr);
        img.ch   = png_get_channels(png_ptr, info_ptr);
        img.it   = png_get_interlace_type(png_ptr, info_ptr);
        rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        printf(" IN: %s : %dx%d\n", filename.c_str(), img.w, img.h);
        img.buf_size = img.h*(rowbytes+1);
        img.buf  = (unsigned char *)malloc(img.buf_size);

        if (png_get_PLTE(png_ptr, info_ptr, &palette, &img.ps))
          memcpy(img.pl, palette, img.ps * 3);
        else
          img.ps = 0;

        if (png_get_tRNS(png_ptr, info_ptr, &trans_alpha, &img.ts, &trans_color))
        {
          if (img.t == PNG_COLOR_TYPE_PALETTE)
            memcpy(img.tr, trans_alpha, img.ts);
          else
            memcpy(&img.tc, trans_color, sizeof(png_color_16));
        }
        else
          img.ts = 0;

        png_set_expand(png_ptr);
        png_set_gray_to_rgb(png_ptr);
        png_set_strip_16(png_ptr);
        png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
        (void)png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);

        img.size = img.h*img.w*4;
        img.image = (png_bytep)malloc(img.size);
        img.frame = (png_bytep)malloc(img.size);
        img.restore = (png_bytep)malloc(img.size);
        img.rows_image = (png_bytepp)malloc(img.h*sizeof(png_bytep));
        img.rows_frame = (png_bytepp)malloc(img.h*sizeof(png_bytep));
 
        if (img.buf && img.image && img.frame && img.restore && img.rows_image && img.rows_frame)
        {
          for (j=0; j<img.h; j++)
            img.rows_image[j] = img.image + j*img.w*4;

          for (j=0; j<img.h; j++)
            img.rows_frame[j] = img.frame + j*img.w*4;

          png_read_image(png_ptr, img.rows_image);
          SavePNG();
          if (img.dop != 0) memset(img.image, 0, img.size);
          png_read_end(png_ptr, info_ptr);

          free(img.rows_frame);
          free(img.rows_image);
          free(img.restore);
          free(img.frame);
          free(img.image);
          free(img.buf);
        }
      }
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }
    inflateEnd(&img.zstream);
    fclose(f1);
  }

  return true;
}

int APNGReader::handle_apng_chunks(png_struct* png_ptr, png_unknown_chunkp chunk)
{
  static int    mask4[2]  = {240,15};
  static int    shift4[2] = {4,0};
  static int    mask2[4]  = {192,48,12,3};
  static int    shift2[4] = {6,4,2,0};
  static int    mask1[8]  = {128,64,32,16,8,4,2,1};
  static int    shift1[8] = {7,6,5,4,3,2,1,0};
  /*unsigned int  seq_num;*/
  image_info  * pimg;

  if (memcmp(chunk->name, "gdEI", 4) == 0)
  {
    if (chunk->size != 70)
      return (-1);

//    pimg = (image_info *)png_get_user_chunk_ptr(png_ptr);
    APNGReader* self = (APNGReader *)png_get_user_chunk_ptr(png_ptr);
    pimg = &(self->img);
    pimg->frames = png_get_uint_31(png_ptr, chunk->data);
    int itemp;
    float ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 1);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->global_domain[0] = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 2);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->global_domain[1] = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 3);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->global_domain[2] = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 4);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->global_domain[3] = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 5);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->global_domain[4] = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 6);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->global_domain[5] = ftemp;
    self->number_scalar_maps  = png_get_uint_31(png_ptr, chunk->data + 4 * 7);
    itemp = png_get_int_32(chunk->data + 4 * 8);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.Position.x = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 9);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.Position.y = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 10);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.Position.z = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 11);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.Focal.x = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 12);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.Focal.y = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 13);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.Focal.z = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 14);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.ViewUp.x = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 15);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.ViewUp.y = ftemp;
    itemp = png_get_int_32(chunk->data + 4 * 16);
    memcpy(&ftemp, &itemp, sizeof(itemp));
    self->camera.ViewUp.z = ftemp;
    unsigned char byte_compression_mode = chunk->data[4 * 17 + 0];
    self->compression_mode = byte_compression_mode;
    unsigned char byte_id_enabled = chunk->data[4 * 17 + 1];
    self->id_enabled = (byte_id_enabled == 0 ? false : true);

    return (1);
  }
  else
  if (memcmp(chunk->name, "ldEI", 4) == 0)
  {
    if (chunk->size != 22)
      return (-1);

    /*seq_num  = png_get_uint_31(png_ptr, chunk->data);*/
//    pimg = (image_info *)png_get_user_chunk_ptr(png_ptr);
    APNGReader* self = (APNGReader *)png_get_user_chunk_ptr(png_ptr);
    pimg = &(self->img);
    pimg->w0 = png_get_uint_31(png_ptr, chunk->data + 4);
    pimg->h0 = png_get_uint_31(png_ptr, chunk->data + 8);
    pimg->x0 = png_get_uint_31(png_ptr, chunk->data + 12);
    pimg->y0 = png_get_uint_31(png_ptr, chunk->data + 16);
    pimg->dop = chunk->data[20];
    pimg->bop = chunk->data[21];
    pimg->zstream.next_out  = pimg->buf;
    pimg->zstream.avail_out = pimg->buf_size;

    return (1);
  }
  else
  if (memcmp(chunk->name, "fdAT", 4) == 0)
  {
    int ret;

    if (chunk->size < 4)
      return (-1);

    /*seq_num = png_get_uint_31(png_ptr, chunk->data);*/
//    pimg = (image_info *)png_get_user_chunk_ptr(png_ptr);
    APNGReader* self = (APNGReader *)png_get_user_chunk_ptr(png_ptr);
    pimg = &(self->img);

    /* interlaced apng - not supported yet */
    if (pimg->it > 0)
      return (0);

    pimg->zstream.next_in   = chunk->data+4;
    pimg->zstream.avail_in  = (uInt)(chunk->size-4);
    ret = inflate(&pimg->zstream, Z_SYNC_FLUSH);

    if (ret == Z_STREAM_END)
    {
      unsigned int    i, j, n;
      unsigned char * sp, * dp;
      unsigned char * prev_row = NULL;
      unsigned char * row = pimg->buf;
      unsigned int    pixeldepth = pimg->d*pimg->ch;
      unsigned int    bpp = (pixeldepth + 7) >> 3;
      unsigned int    rowbytes = (pixeldepth >= 8) ? pimg->w0 * (pixeldepth >> 3) : (pimg->w0 * pixeldepth + 7) >> 3;

      for (j=0; j<pimg->h0; j++)
      {
        /* filters */
        switch (*row++) 
        {
          case 0: break;
          case 1: for (i=bpp; i<rowbytes; i++) row[i] += row[i-bpp]; break;
          case 2: if (prev_row) for (i=0; i<rowbytes; i++) row[i] += prev_row[i]; break;
          case 3: 
            if (prev_row)
            {
              for (i=0; i<bpp; i++) row[i] += prev_row[i]>>1;
              for (i=bpp; i<rowbytes; i++) row[i] += (prev_row[i] + row[i-bpp])>>1;
            } 
            else 
              for (i=bpp; i<rowbytes; i++) row[i] += row[i-bpp]>>1;
            break;
          case 4: 
            if (prev_row) 
            {
              int a, b, c, pa, pb, pc, p;
              for (i=0; i<bpp; i++) row[i] += prev_row[i];
              for (i=bpp; i<rowbytes; i++)
              {
                a = row[i-bpp];
                b = prev_row[i];
                c = prev_row[i-bpp];
                p = b - c;
                pc = a - c;
                pa = abs(p);
                pb = abs(pc);
                pc = abs(p + pc);
                row[i] += ((pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c);
              }
            } 
            else 
              for (i=bpp; i<rowbytes; i++) row[i] += row[i-bpp];
            break;
        }

        /* transform to RGBA 32bpp */
        sp = row;
        dp = pimg->rows_frame[j];
        switch (pimg->t)
        {
          case 0:
            switch (pimg->d)
            {
              case 16: for (i=0; i<pimg->w0; i++) { *dp++ = *sp; *dp++ = *sp; *dp++ = *sp; *dp++ = (pimg->ts && png_get_uint_16(sp)==pimg->tc.gray) ? 0 : 255; sp+=2; }  break;
              case 8:  for (i=0; i<pimg->w0; i++) { *dp++ = *sp; *dp++ = *sp; *dp++ = *sp; *dp++ = (pimg->ts && *sp==pimg->tc.gray) ? 0 : 255; sp++;  }  break;
              case 4:  pimg->tc.gray *= 0x11; for (i=0; i<pimg->w0; i++) { n = ((sp[i>>1] & mask4[i&1]) >> shift4[i&1])*0x11; *dp++ = n; *dp++ = n; *dp++ = n; *dp++ = (pimg->ts && n==pimg->tc.gray) ? 0 : 255; } break;
              case 2:  pimg->tc.gray *= 0x55; for (i=0; i<pimg->w0; i++) { n = ((sp[i>>2] & mask2[i&3]) >> shift2[i&3])*0x55; *dp++ = n; *dp++ = n; *dp++ = n; *dp++ = (pimg->ts && n==pimg->tc.gray) ? 0 : 255; } break;
              case 1:  pimg->tc.gray *= 0xFF; for (i=0; i<pimg->w0; i++) { n = ((sp[i>>3] & mask1[i&7]) >> shift1[i&7])*0xFF; *dp++ = n; *dp++ = n; *dp++ = n; *dp++ = (pimg->ts && n==pimg->tc.gray) ? 0 : 255; } break;
            }
            break;
          case 2:
            switch (pimg->d)
            {
              case 8:  for (i=0; i<pimg->w0; i++, sp+=3) { *dp++ = sp[0]; *dp++ = sp[1]; *dp++ = sp[2]; *dp++ = (pimg->ts && sp[0]==pimg->tc.red && sp[1]==pimg->tc.green && sp[2]==pimg->tc.blue) ? 0 : 255; } break;
              case 16: for (i=0; i<pimg->w0; i++, sp+=6) { *dp++ = sp[0]; *dp++ = sp[2]; *dp++ = sp[4]; *dp++ = (pimg->ts && png_get_uint_16(sp)==pimg->tc.red && png_get_uint_16(sp+2)==pimg->tc.green && png_get_uint_16(sp+4)==pimg->tc.blue) ? 0 : 255; } break;
            }
            break;
          case 3: 
            switch (pimg->d)
            {
              case 8:  for (i=0; i<pimg->w0; i++) { n = sp[i]; *dp++ = pimg->pl[n].red; *dp++ = pimg->pl[n].green; *dp++ = pimg->pl[n].blue; *dp++ = pimg->tr[n]; } break;
              case 4:  for (i=0; i<pimg->w0; i++) { n = (sp[i>>1] & mask4[i&1]) >> shift4[i&1]; *dp++ = pimg->pl[n].red; *dp++ = pimg->pl[n].green; *dp++ = pimg->pl[n].blue; *dp++ = pimg->tr[n]; } break;
              case 2:  for (i=0; i<pimg->w0; i++) { n = (sp[i>>2] & mask2[i&3]) >> shift2[i&3]; *dp++ = pimg->pl[n].red; *dp++ = pimg->pl[n].green; *dp++ = pimg->pl[n].blue; *dp++ = pimg->tr[n]; } break;
              case 1:  for (i=0; i<pimg->w0; i++) { n = (sp[i>>3] & mask1[i&7]) >> shift1[i&7]; *dp++ = pimg->pl[n].red; *dp++ = pimg->pl[n].green; *dp++ = pimg->pl[n].blue; *dp++ = pimg->tr[n]; } break;
            }
            break;
          case 4: 
            switch (pimg->d)
            {
              case 8:  for (i=0; i<pimg->w0; i++) { *dp++ = *sp; *dp++ = *sp; *dp++ = *sp++; *dp++ = *sp++; } break;
              case 16: for (i=0; i<pimg->w0; i++, sp+=4) { *dp++ = sp[0]; *dp++ = sp[0]; *dp++ = sp[0]; *dp++ = sp[2]; } break;
            }
            break;
          case 6:
            switch (pimg->d)
            {
              case 8:  memcpy(dp, sp, pimg->w0*4); break;
              case 16: for (i=0; i<pimg->w0; i++, sp+=8) { *dp++ = sp[0]; *dp++ = sp[2]; *dp++ = sp[4]; *dp++ = sp[6]; } break;
            }
            break;
        }
        prev_row = row;
        row += rowbytes;
      }

      /* prepare for dispose operation */
      if (pimg->dop == 2)
        memcpy(pimg->restore, pimg->image, pimg->size);

      /* blend operation */
      if (pimg->t < 4 && pimg->ts == 0)
        pimg->bop = 0;

      if (pimg->bop == 1)
      {
        int u, v, al;
        for (j=0; j<pimg->h0; j++)
        {
          sp = pimg->rows_frame[j];
          dp = pimg->rows_image[j+pimg->y0] + pimg->x0*4;

          for (i=0; i<pimg->w0; i++, sp+=4, dp+=4)
          {
            if (sp[3] == 255)
              memcpy(dp, sp, 4);
            else
            if (sp[3] != 0)
            {
              if (dp[3] != 0)
              {
                u = sp[3]*255;
                v = (255-sp[3])*dp[3];
                al = 255*255-(255-sp[3])*(255-dp[3]);
                dp[0] = (sp[0]*u + dp[0]*v)/al;
                dp[1] = (sp[1]*u + dp[1]*v)/al;
                dp[2] = (sp[2]*u + dp[2]*v)/al;
                dp[3] = al/255;
              }
              else
                memcpy(dp, sp, 4);
            }
          }  
        }
      }
      else
        for (j=0; j<pimg->h0; j++)
          memcpy(pimg->rows_image[j+pimg->y0] + pimg->x0*4, pimg->rows_frame[j], pimg->w0*4);

      self->SavePNG();

      /* dispose operation */
      if (pimg->dop == 2)
        memcpy(pimg->image, pimg->restore, pimg->size);
      else
      if (pimg->dop == 1)
        for (j=0; j<pimg->h0; j++)
          memset(pimg->rows_image[j+pimg->y0] + pimg->x0*4, 0, pimg->w0*4);

      inflateReset(&pimg->zstream);
    }
    else
    if (ret != Z_OK)
      return (-1);

    return (1);
  }
  else
    return (0);
}

void APNGReader::SavePNG()
{
    int mImagesPerFrame = 0;
    switch (compression_mode)
    {
    case 1:
        mImagesPerFrame = 2;
        break;
    case 2:
    case 4:
        mImagesPerFrame = 3;
        break;
    case 3:
        mImagesPerFrame = 4;
        break;
    default:
        assert(false);
    }
    if (id_enabled)
        ++mImagesPerFrame;

    int frame_index = image_index / mImagesPerFrame;
    int buffer_index = image_index % mImagesPerFrame;
    if (buffer_index == 0)
    {
        Frame* f = new Frame();
        f->SetNumberOfScalarMaps(id_enabled ? number_scalar_maps : number_scalar_maps + 1);
        f->SetSize(img.w, img.h);
        double domain[6];
        for (int i = 0; i < 6; ++i)
        {
            domain[i] = global_domain[i];
        }
//        domain[0] = -310; domain[1] = 310; domain[2] = -310; domain[3] = 310; domain[4] = 690; domain[5] = 1310;
        f->SetDataDomain(domain);
        frames.push_back(f);
    }
    Frame* f = frames[frame_index];
    switch (buffer_index)
    {
    case 0:
        f->SetColorMap(img.image);
        break;
    case 1:
        if (compression_mode == 1 || compression_mode == 4)
        {
            float* depth = new float [img.w * img.h];
            float* normal = new float [img.w * img.h * 3];
            divDepthNormal(&img, depth, normal);
            f->SetDepthMap(depth);
            f->SetNormalMap(normal);
            delete [] depth;
            delete [] normal;
        } else
        {
            float* depth = new float [img.w * img.h];
            for (unsigned int i = 0; i < img.w * img.h; ++i)
                depth[i] = (reinterpret_cast<float*>(img.image)[i] - global_domain[4]) / (global_domain[5] - global_domain[4]);
            f->SetDepthMap(depth);
        }
        break;
    case 2:
        if (compression_mode == 2 || compression_mode == 3)
        {
            float* normal = new float [img.w * img.h * 3];
            convertNormal(&img, normal);
            f->SetNormalMap(normal);
            delete [] normal;
        } else
            f->SetScalarMap(0, reinterpret_cast<float *>(img.image));
        break;
    case 3:
        assert(((compression_mode == 2 || compression_mode == 4) && id_enabled) || compression_mode == 3);
        if (compression_mode == 2 || compression_mode == 3)
            f->SetScalarMap(0, reinterpret_cast<float *>(img.image));
        else if (compression_mode == 4)
            f->SetScalarMap(1, reinterpret_cast<float *>(img.image));
        break;
    case 4:
        assert(compression_mode == 3 && id_enabled);
        f->SetScalarMap(1, reinterpret_cast<float *>(img.image));
        break;
    }

    ++image_index;
}

void APNGReader::divDepthNormal(image_info *pimg, float *depthMap, float *normalMap)
{
    for (unsigned int i = 0; i < pimg->w * pimg->h; ++i)
    {
        float* image = reinterpret_cast<float*>(pimg->image);
        float depthNormal = image[i];
        unsigned char temp[4];
        memcpy(temp, &depthNormal, 4);
        unsigned short usdepth;
        memcpy(&usdepth, temp, 2);
        float depth = float(usdepth) / USHRT_MAX;
        unsigned char theta = temp[2];
        unsigned char lamda = temp[3];
        unsigned short ustheta = float(theta) / float(UCHAR_MAX) * float(USHRT_MAX);
        unsigned short uslamda = float(lamda) / float(UCHAR_MAX) * float(USHRT_MAX);
        float normal[3];
        normal[0] = 0.f; normal[1] = 0.f; normal[2] = 0.f;
        if (!(ustheta == 0 && uslamda == 0))
        {
            float thetaF = float(theta) / USHRT_MAX * 2 * M_PI - M_PI;
            float lamdaF = float(uslamda) / USHRT_MAX * 2 * M_PI - M_PI;
            float p = 1.0;
            float S = p * cos(lamdaF);
            normal[0] = p * sin(lamdaF);
            normal[1] = S * sin(thetaF);
            normal[2] = S * cos(thetaF);
        }
        depthMap[i] = depth;
        normalMap[i * 3 + 0] = normal[0];
        normalMap[i * 3 + 1] = normal[1];
        normalMap[i * 3 + 2] = normal[2];
    }
}

void APNGReader::convertNormal(image_info *pimg, float *normalMap)
{
    for (unsigned int i = 0; i < pimg->w * pimg->h; ++i)
    {
        unsigned short* image = reinterpret_cast<unsigned short*>(pimg->image);
        unsigned short theta = image[2 * i + 0];
        unsigned short lamda = image[2 * i + 1];
        float thetaF = float(theta) / USHRT_MAX * 2 * M_PI - M_PI;
        float lamdaF = float(lamda) / USHRT_MAX * 2 * M_PI - M_PI;
        float p = 1.0;
        float S = p * cos(lamdaF);
        normalMap[i * 3 + 0] = p * sin(lamdaF);
        normalMap[i * 3 + 1] = S * sin(thetaF);
        normalMap[i * 3 + 2] = S * cos(thetaF);
    }
}
