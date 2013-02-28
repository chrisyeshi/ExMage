#ifndef __CoreTube_h
#define __CoreTube_h

#include "Particle.h"

#define USE_OSMESA

#ifdef USE_OSMESA
  #include "GL/osmesa.h"
#else
  #include "GL/gl.h"
  #include "GL/glx.h"
#endif

#include <vector>

#include "TransferFunction.h"
#include "CameraCore.h"
#include "Geometry/point.h"

#define WINDOW_WIDTH 128
#define WINDOW_HEIGHT 128
#define RADIUS 2.0
#define MAX_PARTICLE_GAP 5.0

class Frame;

class CoreTube
{
public:
  CoreTube();
  ~CoreTube();

  void SetCameras(const std::vector<CameraCore>& cameras);
  void SetLightPosition(const Point& light) {LightPosition = light;};
  void SetExtent(double extent[6]);
  void GenerateTubes(const std::vector<tube::Particle>& particles1, const std::vector<tube::Particle>& particles2);
  int GetCameraCount() const {return Frames.size();};
  Frame* GetFrame(const int index);// {return Frames[index];};

protected:
  double Extent[6];
#ifdef USE_OSMESA
  OSMesaContext Ctx;
  GLubyte* ColorBuffer;
//  float* DepthBuffer;
#else
  Display* dpy;
  GLXContext ctx;
#endif
  TransferFunction Tf;
  int CurCamIndex;
  Point LightPosition;
  std::vector<Frame*> Frames;
  GLuint DisplayListIndex;
  GLuint* Fbo;
  // 0 => depth :: 1 => color :: 2 => normal :: 3 => MF :: 4 => temperature :: 5 => ID
  GLuint* BufTex;
  GLhandleARB VertShader[5];
  GLhandleARB FragShader[5];
  GLhandleARB ProgShader[5];
  int BufferIndex;

  void loadConfigureFile();
  void createDisplayList(const std::vector<tube::Particle>& particles1, const std::vector<tube::Particle>& particles2);
  void deleteDisplayList();
  void initGLContext();
  void initGLEnv();
  void initFBO();
  void initDepthTexture(int texture_index);
  void initColorTexture(int texture_index);
  void initNormalTexture(int texture_index);
  void initScalarTexture(int texture_index);
  void initTF();
  void initShaders();
  GLhandleARB readShader(GLenum shader_type, const std::string& path);
  void printInfoLog(GLhandleARB obj);
  void setupCamera();
  void setupLightEnv();
  void drawTube(const tube::Particle& p1, const tube::Particle& p2);
  void drawSphere(const tube::Particle& p1, const tube::Particle& p2);
  void snapshot();
  void mvMult(double matrix[16], double vec[4], double out[4]) const;
  void calcDomain();
};

#endif
