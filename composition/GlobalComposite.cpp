#include "GlobalComposite.h"

#include <iostream>
#include <string>
#include <cassert>
#include <cfloat>
#include <climits>
#include <cstring>

#include "mpi.h"
#include "APNGWriter.h"

GlobalComposite::GlobalComposite() : depth_list(NULL)
{
    global_resolution[0] = global_resolution[1] = 0;
    global_extent[0] = global_extent[2] = global_extent[4] = FLT_MAX;
    global_extent[1] = global_extent[3] = global_extent[5] = -FLT_MAX;
}

GlobalComposite::~GlobalComposite()
{
    for (unsigned int i = 0; i < frames.size(); ++i)
        delete frames[i];
    if (depth_list)
        delete [] depth_list;
}

void GlobalComposite::gather(Frame* leaf)
{
    double leaf_domain[6];
    leaf->GetDataDomain(leaf_domain);
    // Let's say for now we only support 1 camera
    // And Let's gather each buffer separatly... (stupid)
    // Assume we only have 1 scalar map
    // Individual buffer size
    int leafSize[2];
    leaf->GetSize(leafSize);
    int nPixels = leafSize[0] * leafSize[1];
    // Camera
    CameraCore camera = leaf->GetCamera();
    // Initialize gathered array of frames
    // if this is root
    int rank, nProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    assert(frames.empty());
    if (rank == root)
    {
        frames.resize(nProcs);
        for (unsigned int i = 0; i < frames.size(); ++i)
        {
            frames[i] = new Frame;
            frames[i]->SetNumberOfScalarMaps(NUM_SCALARS);
            frames[i]->SetSize(leafSize);
            frames[i]->SetCamera(camera);
        }
    }
    // Gather domain
    double domains[6 * nProcs];
    double domain[6];
    leaf->GetDataDomain(domain);
    MPI_Gather(domain, 6, MPI_DOUBLE,
            domains, 6, MPI_DOUBLE, root, MPI_COMM_WORLD);
    for (unsigned int i = 0; i < frames.size(); ++i)
        frames[i]->SetDataDomain(&domains[i * 6]);
    // Gather depth map
    float* depthBuffer = new float [nPixels * nProcs];
    MPI_Gather(leaf->GetDepthMap(), nPixels, MPI_FLOAT,
            depthBuffer, nPixels, MPI_FLOAT, root, MPI_COMM_WORLD);
    for (unsigned int i = 0; i < frames.size(); ++i)
        frames[i]->SetDepthMap(&depthBuffer[i * nPixels]);
    delete [] depthBuffer;
    // Gather color map
    unsigned char* colorBuffer = new unsigned char [4 * nPixels * nProcs];
    MPI_Gather(leaf->GetColorMap(), 4 * nPixels, MPI_UNSIGNED_CHAR,
            colorBuffer, 4 * nPixels, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
    for (unsigned int i = 0; i < frames.size(); ++i)
        frames[i]->SetColorMap(&colorBuffer[i * 4 * nPixels]);
    delete [] colorBuffer;
    // Gather normal map
    unsigned char* normalBuffer = new unsigned char [4 * nPixels * nProcs];
    MPI_Gather(leaf->GetNormalMap(), 4 * nPixels, MPI_UNSIGNED_CHAR,
            normalBuffer, 4 * nPixels, MPI_UNSIGNED_CHAR, root, MPI_COMM_WORLD);
    for (unsigned int i = 0; i < frames.size(); ++i)
        frames[i]->SetNormalMap(&normalBuffer[i * 4 * nPixels]);
    delete [] normalBuffer;
    // Gather scalar map
    float* scalarBuffer = new float [nPixels * nProcs];
    MPI_Gather(leaf->GetScalarMap(0), nPixels, MPI_FLOAT,
            scalarBuffer, nPixels, MPI_FLOAT, root, MPI_COMM_WORLD);
    for (unsigned int i = 0; i < frames.size(); ++i)
        frames[i]->SetScalarMap(0, &scalarBuffer[i * nPixels]);
    delete [] scalarBuffer;
}

void GlobalComposite::composite()
{
    // if not root, do nothing
    int rank, nProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    if (rank != root)
        return;
    // make sure the frames array is not empty (run gather() first?)
    if (frames.empty())
        return;
    // calculate global_extent
    global_extent[0] = global_extent[2] = global_extent[4] = FLT_MAX;
    global_extent[1] = global_extent[3] = global_extent[5] = -FLT_MAX;
    int frame_size[2];
    double frame_domain[6];
    for (unsigned int i = 0; i < frames.size(); ++i)
    {
        frames[i]->GetSize(frame_size);
        frames[i]->GetDataDomain(frame_domain);
        for (int j = 0; j < 3; ++j)
        {
            global_extent[j * 2] = std::min(global_extent[j * 2], frame_domain[j * 2]);
            global_extent[j * 2 + 1] = std::max(global_extent[j * 2 + 1], frame_domain[j * 2 + 1]);
        }
    }
    std::cout << "Global Extent: [" << global_extent[0] << ", " << global_extent[1] << ", " << global_extent[2] << ", " << global_extent[3] << ", " << global_extent[4] << ", " << global_extent[5] << "]" << std::endl;
    // calculate the total resolution
    std::cout << "individual resolution " << frame_size[0] << " " << frame_size[1] << std::endl;
    global_resolution[0] = double(frame_size[0]) * (global_extent[1] - global_extent[0]) / (frame_domain[1] - frame_domain[0]);
    global_resolution[1] = double(frame_size[1]) * (global_extent[3] - global_extent[2]) / (frame_domain[3] - frame_domain[2]);
    std::cout << "estimated actual resolution " << global_resolution[0] << " " << global_resolution[1] << std::endl;
    // initialize the global image space
    if (depth_list)
        delete [] depth_list;
    depth_list = new std::map<float, PixelInfo> [global_resolution[0] * global_resolution[1]];
    int max_depth_level = 0;
    // loop through each pixel and add to the global space
    for (unsigned int i = 0; i < frames.size(); ++i)
    {
        int size[2];
        frames[i]->GetSize(size);
        double domain[6];
        frames[i]->GetDataDomain(domain);
        for (int y = 0; y < size[1]; ++y)
        {
            for (int x = 0; x < size[0]; ++x)
            {
                int frame_pixel_index = size[0] * y + x;
                float depth = frames[i]->GetDepthMap()[frame_pixel_index];
                if (depth > 0.9999 || depth < 0.0001)
                    continue;
                float zz = depth * (domain[5] - domain[4]) + domain[4];
                float nxx = float(x) / size[0];
                float xx = nxx * (domain[1] - domain[0]) + domain[0] - global_extent[0];
                int ixx = int(xx * global_resolution[0] / (global_extent[1] - global_extent[0]) + 0.5);
                float nyy = float(y) / size[1];
                float yy = nyy * (domain[3] - domain[2]) + domain[2] - global_extent[2];
                int iyy = int(yy * global_resolution[1] / (global_extent[3] - global_extent[2]) + 0.5);
                int pixel_index = global_resolution[0] * iyy + ixx;
                // idata: x, y, id
                // fdata: z, color, normal, scalar
                PixelInfo info(3, 2, 7);
                info.idata[0] = ixx;
                info.idata[1] = iyy;
                info.idata[2] = int(frames[i]->GetScalarMap(0)[frame_pixel_index] * 128000.0);
                info.fdata[0] = zz;
                info.fdata[1] = frames[i]->GetScalarMap(0)[frame_pixel_index];
                info.udata[0] = frames[i]->GetColorMap()[4 * frame_pixel_index + 0];
                info.udata[1] = frames[i]->GetColorMap()[4 * frame_pixel_index + 1];
                info.udata[2] = frames[i]->GetColorMap()[4 * frame_pixel_index + 2];
                info.udata[3] = frames[i]->GetColorMap()[4 * frame_pixel_index + 3];
                info.udata[4] = frames[i]->GetNormalMap()[4 * frame_pixel_index + 0];
                info.udata[5] = frames[i]->GetNormalMap()[4 * frame_pixel_index + 1];
                info.udata[6] = frames[i]->GetNormalMap()[4 * frame_pixel_index + 2];

                std::map<float, PixelInfo>& pixel_depth_list = depth_list[pixel_index];
                // if it is the first element at this pixel
                if (pixel_depth_list.empty())
                {
                    pixel_depth_list[zz] = info;
                    max_depth_level = std::max(int(pixel_depth_list.size()), max_depth_level);
                    continue;
                }
                // if it's very close to the depth in front of it
                std::map<float, PixelInfo>::iterator behind = pixel_depth_list.lower_bound(zz);
                std::map<float, PixelInfo>::iterator front = behind;
                if (front != pixel_depth_list.begin())
                    front--;
                if (front != pixel_depth_list.begin() &&
                    fabs((*front).first - zz) < 10.0)
                {
                    continue;
                }
                // if it's very close to the depth behind it
                if (behind != pixel_depth_list.end() &&
                    fabs(zz - (*behind).first) < 10.0)
                {
                    pixel_depth_list.erase(behind);
                    pixel_depth_list[zz] = info;
                    continue;
                }
                // otherwise
                pixel_depth_list[zz] = info;
                max_depth_level = std::max(int(pixel_depth_list.size()), max_depth_level);
            }
        }
// delete frames[i];
    }
    std::cout << "done reading" << std::endl;
// output it back
    APNGWriter writer;
    writer.setFileName("apng.png");
    writer.setSize(global_resolution);
    writer.setGlobalDomain(global_extent);
    writer.setNumberOfScalarMaps(NUM_SCALARS);
    writer.setCamera(frames[0]->GetCamera());
    writer.setCompressionMode(compressMode);
    writer.setIdEnabled(enableId);
    for (int depth_level = 0; depth_level < max_depth_level; ++depth_level)
    {
        std::cout << "depth level " << depth_level << std::endl;
// buffer
        unsigned char* color = new unsigned char [4 * global_resolution[0] * global_resolution[1]];
        unsigned short* normal = new unsigned short [2 * global_resolution[0] * global_resolution[1]];
        float* depth = new float [global_resolution[0] * global_resolution[1]];
        unsigned int* depthnormal = new unsigned int [global_resolution[0] * global_resolution[1]];
        float* scalar = new float [global_resolution[0] * global_resolution[1]];
        float* id = new float [global_resolution[0] * global_resolution[1]];

        for (int y = 0; y < global_resolution[1]; ++y)
        {
            for (int x = 0; x < global_resolution[0]; ++x)
            {
                int index = y * global_resolution[0] + x;
                std::map<float, PixelInfo>& pixel_depth_list = depth_list[index];
                if (pixel_depth_list.empty())
                {
                    color[4 * index + 0] = 0;
                    color[4 * index + 1] = 0;
                    color[4 * index + 2] = 0;
                    color[4 * index + 3] = 0;
                    normal[2 * index + 0] = 0;
                    normal[2 * index + 1] = 0;
                    depth[index] = 0.0;
                    depthnormal[index] = 0;
                    scalar[index] = 0.0;
                    id[index] = 0.0;
                    continue;
                }

                PixelInfo info = pixel_depth_list.begin()->second;
                pixel_depth_list.erase(pixel_depth_list.begin());
                color[4 * index + 0] = info.udata[0];
                color[4 * index + 1] = info.udata[1];
                color[4 * index + 2] = info.udata[2];
                color[4 * index + 3] = info.udata[3];

                unsigned short spherical_normal[2];
                convertNormal(info.udata[4], info.udata[5], info.udata[6], spherical_normal);

                normal[2 * index + 0] = spherical_normal[0];
                normal[2 * index + 1] = spherical_normal[1];

                depth[index] = info.fdata[0];
                depthnormal[index] = combineDepthNormal(info.fdata[0], info.udata[4], info.udata[5], info.udata[6]);
                scalar[index] = info.fdata[1];
                id[index] = info.idata[2] / 128000.0;
            }
        }

        writer.addImage(color);
        switch (compressMode)
        {
            case 1:
            writer.addImage(depthnormal);
            break;
            case 2:
            writer.addImage(depth);
            writer.addImage(normal);
            break;
            case 3:
            writer.addImage(depth);
            writer.addImage(normal);
            writer.addImage(scalar);
            break;
            case 4:
            writer.addImage(depthnormal);
            writer.addImage(scalar);
            break;
            default:
            assert(false);
        }
        if (enableId)
            writer.addImage(id);

        Frame frame;
        char filename[20];
        sprintf(filename, "depth_level_%d", depth_level);
        frame.SetFileName(filename);
        frame.SetSize(global_resolution);
        frame.SetDataDomain(global_extent);
        frame.SetNumberOfScalarMaps(NUM_SCALARS);
        frame.SetColorMap(color);
        frame.SetDepthMap(depth);
        frame.SetNormalMap(reinterpret_cast<unsigned char *>(normal));
        //    frame.SetScalarMap(0, scalar);
        frame.SetScalarMap(0, id);
        frame.Write();

        /*
        delete [] color;
        delete [] normal;
        delete [] depth;
        delete [] scalar;
        delete [] id;
        */
    }
    writer.write();
}

void GlobalComposite::convertNormal(unsigned char x, unsigned char y, unsigned char z, unsigned short spherical[2]) const
{
  float fx, fy, fz;
  fx = float(x) / UCHAR_MAX * 2.0 - 1.0;
  fy = float(y) / UCHAR_MAX * 2.0 - 1.0;
  fz = float(z) / UCHAR_MAX * 2.0 - 1.0; 
  if (fz < 0.0)
  {
//    std::cout << "normal: " << fx << ", " << fy << ", " << fz << std::endl;
    spherical[0] = 0;
    spherical[1] = 0;
    return;
  }
  float sphericalF[2];
  convertNormalF(fx, fy, fz, sphericalF);
  for (int i = 0; i < 2; ++i)
  {
    float normalize_spherical = (sphericalF[i] - (-M_PI)) / (2.0 * M_PI);
    if (normalize_spherical < 0.0 || normalize_spherical > 1.0)
      std::cout << "normalize spherical: " << normalize_spherical << std::endl;
    spherical[i] = normalize_spherical * USHRT_MAX;
  }
}

void GlobalComposite::convertNormalF(float x, float y, float z, float spherical[2]) const
{
//  if (x > 0.0)
//    printf("normal: %2.2f, %2.2f, %2.2f\n", x, y, z);
  float S = sqrt(y * y + z * z);
  float p = sqrt(x * x + y * y + z * z);
  float theta = asin(y / S);
  float lamda = asin(x / p);
//  if (theta > 1.5 || lamda > 1.5 || theta < 0.0 || lamda < 0.0)
//    printf("theta = %2.2f, lamda = %2.2f\n", theta, lamda);
  spherical[0] = theta;
  spherical[1] = lamda;
//  std::cout << "original angles: " << spherical[0] << ", " << spherical[1] << std::endl;
//  std::cout << "xyz: " << x << ", " << y << ", " << z << std::endl;
}

unsigned int GlobalComposite::combineDepthNormal(float depth, unsigned char x, unsigned char y, unsigned char z) const
{
  unsigned short usdepth = (depth - global_extent[4]) / (global_extent[5] - global_extent[4]) * USHRT_MAX;
  unsigned short spherical[2];
  convertNormal(x, y, z, spherical);
  unsigned char usnormal[2];
  for (int i = 0; i < 2; ++i)
  {
    usnormal[i] = float(spherical[i]) / float(USHRT_MAX) * float(UCHAR_MAX);
//    printf("%d, ", usnormal[i]);
  }
  unsigned char temp[4];
  memcpy(temp, &usdepth, 2);
  temp[2] = usnormal[0];
  temp[3] = usnormal[1];
//  memcpy(&(temp[2]), &usnormal, 2);
  unsigned int ret = 0;
  memcpy(&ret, temp, 4);
//  std::cout << "    depth: " << ret;
//  std::cout << "    depthnormal: " << ret << std::endl;
  return ret;
}
