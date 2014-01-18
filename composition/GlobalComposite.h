#ifndef __GLOBALCOMPOSITE_H_
#define __GLOBALCOMPOSITE_H_

#include <vector>
#include <map>

#include "Frame.h"

class PixelInfo
{
public:
    PixelInfo() {};
    PixelInfo(int isize, int fsize, int usize) {idata.resize(isize); fdata.resize(fsize); udata.resize(usize);};
    std::vector<int> idata;
    std::vector<float> fdata;
    std::vector<unsigned char> udata;
};

class GlobalComposite
{
public:
    GlobalComposite();
    virtual ~GlobalComposite();

    void gather(Frame* leaf);
    void composite();

protected:
    void convertNormal(unsigned char x, unsigned char y, unsigned char z, unsigned short spherical[2]) const;
    void convertNormalF(float x, float y, float z, float spherical[2]) const;
    unsigned int combineDepthNormal(float depth, unsigned char x, unsigned char y, unsigned char z) const;

private:
    static const int NUM_SCALARS = 1;
    static const int root = 0;
    static const int nCameras = 1;
    static const int compressMode = 4;
    static const bool enableId = false;

    int global_resolution[2];
    double global_extent[6];
    std::vector<Frame *> frames;
    std::map<float, PixelInfo>* depth_list;
};

#endif
