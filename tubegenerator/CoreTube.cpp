#define GL_GLEXT_PROTOTYPES
#include "CoreTube.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glx.h"
#include "GL/glext.h"

#include <iostream>
#include <cassert>
#include <cfloat>
#include <fstream>

#include "Frame.h"
#include "vector.h"
#include "PNGWriter.h"
#include "ConfigReader.h"
#include "ProcIndex.h"
#include "mkpath.h"

#ifdef USE_OSMESA
#else
typedef GLXContext (*glXCreateContextAttribsARBProc)
                   (Display*, GLXFBConfig, GLXContext, Bool, const int*);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
typedef Bool (*glXMakeContextCurrentARBProc)
             (Display*, GLXDrawable, GLXDrawable, GLXContext);
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;
#endif

CoreTube::CoreTube()
{
}

CoreTube::~CoreTube()
{
//  glDeleteShader(VertShader);
//  glDeleteShader(FragShader);
//  glDeleteProgram(ProgShader);

  glDeleteFramebuffersEXT(5 * Frames.size(), Fbo);
  glDeleteTextures(2 * 5 * Frames.size(), BufTex);
  delete [] Fbo;
  delete [] BufTex;
  for (unsigned int i = 0; i < Frames.size(); ++i)
    delete Frames[i];
#ifdef USE_OSMESA
  delete [] ColorBuffer;
  OSMesaDestroyContext(Ctx);
#else
  glXDestroyContext(dpy, ctx);
#endif
}

void CoreTube::Initialize()
{
//  loadConfigureFile();
  initGLContext();
  initGLEnv();
  initShaders();
  initTF();
}

void CoreTube::SetCameras(const std::vector<CameraCore>& cameras)
{
  for (unsigned int i = 0; i < Frames.size(); ++i)
    delete Frames[i];
  Frames.resize(cameras.size());
  for (unsigned int i = 0; i < Frames.size(); ++i)
  {
    Frames[i] = new Frame();
    Frames[i]->SetCamera(cameras[i]);
  }
  initFBO();
}

void CoreTube::SetExtent(double extent[6])
{
  for (int i = 0; i < 6; ++i)
    Extent[i] = extent[i];
}

void CoreTube::GenerateTubes(const std::vector<Particle<> >& particles1,
                             const std::vector<Particle<> >& particles2)
{
  clock_t start, end, tick;
  start = clock();
//  std::cout << "particles2: " << particles2.size() << "\n";
  for (CurCamIndex = 0; CurCamIndex < Frames.size(); ++CurCamIndex)
  {
    for (BufferIndex = 0; BufferIndex < 5; ++BufferIndex)
    {
      glUseProgramObjectARB(ProgShader[BufferIndex]);

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fbo[5 * CurCamIndex + BufferIndex]);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_1D, Tf.GetTexture());
      glEnable(GL_TEXTURE_1D);
      //  createDisplayList(particles1, particles2);
      setupCamera();
      setupLightEnv();
      //    glCallList(DisplayListIndex);
      for (unsigned int i = 0; i < particles1.size(); ++i)
      {
        Point p1(double(particles1[i].x()), double(particles1[i].y()), double(particles1[i].z()));
        Point p2(double(particles2[i].x()), double(particles2[i].y()), double(particles2[i].z()));
        if (d(p1, p2) > max_particle_gap())
          continue;
        drawTube(particles1[i], particles2[i]);
        drawSphere(particles1[i], particles2[i]);
        //      drawSphere(particles2[i], particles1[i]);

//        glFinish();
        //snapshot();
      }
      //  deleteDisplayList();
      glDisable(GL_TEXTURE_1D);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_1D, 0);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      glUseProgramObjectARB(0);
    }
  }

}

Frame* CoreTube::GetFrame(const int index)
{
  CurCamIndex = index;
  snapshot();
  return Frames[index];
}

void CoreTube::Output()
{
    ProcIndex procindex;
    int rank = procindex.getGlobalIndex();
    std::cout << "Proc: " << rank << " Progress: Saving..." << std::endl;
    for (int i = 0; i < GetCameraCount(); ++i)
    {
        Frame* sum = GetFrame(i);
        char proc_index_string[10];
        sprintf(proc_index_string, "%02d", rank);
        char cam_index_string[10];
        sprintf(cam_index_string, "%02d", i);

        char pcs[100];
        int resolution[2];
        sum->GetSize(resolution);
        sprintf(pcs, "n%d_p%d_r%d_t%d",
                config().GetRegionCount()[0],
                config().GetRegionParticleCount(),
                resolution[0],
                config().GetTimeStepRange()[1] - config().GetTimeStepRange()[0]);

        std::string dir = config().GetOutRoot() + "/" + pcs
                        + std::string("/cam") + cam_index_string;
        std::string spt_path = dir + "/output_proc" + proc_index_string;
        mkpath(dir.c_str(), 0777);
        mkpath(dir.c_str(), 0777);
        mkpath(dir.c_str(), 0777);
        sum->SetFileName(spt_path.c_str());
        sum->Write();
    }
    // output times
    Frame* sum = GetFrame(0);
    char pcs[100];
    int resolution[2];
    sum->GetSize(resolution);
    sprintf(pcs, "n%d_p%d_r%d_t%d",
            config().GetRegionCount()[0],
            config().GetRegionParticleCount(),
            resolution[0],
            config().GetTimeStepRange()[1] - config().GetTimeStepRange()[0]);

    std::string dir = config().GetOutRoot() + "/" + pcs + "/time";
    char global_index_string[10];
    sprintf(global_index_string, "%02d", rank);
    std::string time_path = dir + "/proc" + global_index_string + ".txt";
    mkpath(dir.c_str(), 0777);
    mkpath(dir.c_str(), 0777);
    mkpath(dir.c_str(), 0777);
    std::ofstream tout(time_path.c_str());
    for (unsigned int i = 0; i < times.size(); ++i)
    {
        tout << times[i] << ",";
    }
}

int CoreTube::window_width() const
{
    return config().GetResolution()[0];
}

int CoreTube::window_height() const
{
    return config().GetResolution()[1];
}

float CoreTube::radius() const
{
    return config().GetTubeRadius();
}

float CoreTube::max_particle_gap() const
{
    return config().GetMaxParticleGap();
}

void CoreTube::createDisplayList(const std::vector<Particle<> >& particles1,
                                 const std::vector<Particle<> >& particles2)
{
  DisplayListIndex = glGenLists(1);
  glNewList(DisplayListIndex, GL_COMPILE);
    for (unsigned int i = 0; i < particles1.size(); ++i)
    {
      Point p1(double(particles1[i].x()), double(particles1[i].y()), double(particles1[i].z()));
      Point p2(double(particles2[i].x()), double(particles2[i].y()), double(particles2[i].z()));
      if (d(p1, p2) > max_particle_gap())
        continue;
      drawTube(particles1[i], particles2[i]);
      drawSphere(particles1[i], particles2[i]);
//      drawSphere(particles2[i], particles1[i]);
    }
  glEndList();
}

void CoreTube::deleteDisplayList()
{
  glDeleteLists(DisplayListIndex, 1);
}

void CoreTube::initGLContext()
{
#ifdef USE_OSMESA
//  Ctx = OSMesaCreateContextExt(OSMESA_RGBA, 32, 0, 0, NULL);
//  ColorBuffer = new GLubyte [WINDOW_WIDTH * WINDOW_HEIGHT * 4];
//  DepthBuffer = new float [WINDOW_WIDTH * WINDOW_HEIGHT];
//  OSMesaMakeCurrent(Ctx, ColorBuffer, GL_UNSIGNED_BYTE, WINDOW_WIDTH, WINDOW_HEIGHT);

   /* Create an RGBA-mode context */
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
   /* specify Z, stencil, accum sizes */
   Ctx = OSMesaCreateContextExt( OSMESA_RGBA, 32, 0, 0, NULL );
#else
   Ctx = OSMesaCreateContext( OSMESA_RGBA, NULL );
#endif
   if (!Ctx) {
      printf("OSMesaCreateContext failed!\n");
      return;
   }

   /* Allocate the image buffer */
   ColorBuffer = new GLubyte [window_area() * 4];
//   buffer = malloc( Width * Height * 4 * sizeof(GLubyte) );
   if (!ColorBuffer) {
      printf("Alloc image buffer failed!\n");
      return;
   }

   /* Bind the buffer to the context and make it current */
   if (!OSMesaMakeCurrent( Ctx, ColorBuffer, GL_UNSIGNED_BYTE, window_width(), window_height() )) {
      printf("OSMesaMakeCurrent failed!\n");
      return;
   }

   {
      int z, s, a;
      glGetIntegerv(GL_DEPTH_BITS, &z);
      glGetIntegerv(GL_STENCIL_BITS, &s);
      glGetIntegerv(GL_ACCUM_RED_BITS, &a);
//      printf("Depth=%d Stencil=%d Accum=%d\n", z, s, a);
   }

#else
  static int visual_attribs[] = {None};
  int context_attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                           GLX_CONTEXT_MINOR_VERSION_ARB, 0,
                           None};

  dpy = XOpenDisplay(0);
  int fbcount = 0;
  GLXFBConfig* fbc = NULL;
//  GLXContext ctx;
  GLXPbuffer pbuf;

  fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount);

  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
  glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");

  ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs);

  int pbuffer_attribs[] = {GLX_PBUFFER_WIDTH, WINDOW_WIDTH,
                           GLX_PBUFFER_HEIGHT, WINDOW_HEIGHT,
                           None};
  pbuf = glXCreatePbuffer(dpy, fbc[0], pbuffer_attribs);

  XFree(fbc);
  XSync(dpy, False);

  glXMakeContextCurrentARB(dpy, pbuf, pbuf, ctx);
#endif
}

void CoreTube::initGLEnv()
{
//  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

//  glEnable(GL_LIGHTING);
//  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE_ARB);
//  glEnable(GL_COLOR_MATERIAL);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CoreTube::initFBO()
{
  GLint maxbuffers;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxbuffers);
//  std::cout << "Max Buffer: " << maxbuffers << std::endl;

  Fbo = new GLuint [Frames.size() * 5];
  BufTex = new GLuint [Frames.size() * 5 * 2];

  glGenFramebuffersEXT(Frames.size() * 5, Fbo);
  glGenTextures(Frames.size() * 5 * 2, BufTex);

  for (unsigned int i = 0; i < Frames.size(); ++i)
  {
      int j;
      // color
      j = 0;
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fbo[5 * i + j]);
      initDepthTexture(2 * 5 * i + 2 * j + 0);
      initColorTexture(2 * 5 * i + 2 * j + 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      // normal
      j = 1;
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fbo[5 * i + j]);
      initDepthTexture(2 * 5 * i + 2 * j + 0);
      initNormalTexture(2 * 5 * i + 2 * j + 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      // mixture fraction
      j = 2;
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fbo[5 * i + j]);
      initDepthTexture(2 * 5 * i + 2 * j + 0);
      initScalarTexture(2 * 5 * i + 2 * j + 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      // temperature texture
      j = 3;
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fbo[5 * i + j]);
      initDepthTexture(2 * 5 * i + 2 * j + 0);
      initScalarTexture(2 * 5 * i + 2 * j + 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      // ID texture
      j = 4;
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Fbo[5 * i + j]);
      initDepthTexture(2 * 5 * i + 2 * j + 0);
      initScalarTexture(2 * 5 * i + 2 * j + 1);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

/*
      GLenum buffers[] = {GL_COLOR_ATTACHMENT0_EXT,
                          GL_COLOR_ATTACHMENT1_EXT,
                          GL_COLOR_ATTACHMENT2_EXT,
                          GL_COLOR_ATTACHMENT3_EXT,
                          GL_COLOR_ATTACHMENT4_EXT};
      glDrawBuffers(5, buffers);
*/
  }
}

void CoreTube::initDepthTexture(int texture_index)
{
  glBindTexture(GL_TEXTURE_2D, BufTex[texture_index]); // depth
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
                 window_width(), window_height(), 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                GL_TEXTURE_2D, BufTex[texture_index], 0);
}

void CoreTube::initColorTexture(int texture_index)
{
  glBindTexture(GL_TEXTURE_2D, BufTex[texture_index]); // color
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 window_width(), window_height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D, BufTex[texture_index], 0);
}

void CoreTube::initNormalTexture(int texture_index)
{
  glBindTexture(GL_TEXTURE_2D, BufTex[texture_index]); // normal
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8,
                 window_width(), window_height(), 0,
                 GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D, BufTex[texture_index], 0);
}

void CoreTube::initScalarTexture(int texture_index)
{
  glBindTexture(GL_TEXTURE_2D, BufTex[texture_index]); // mixture fraction
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 window_width(), window_height(), 0,
                 GL_LUMINANCE, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                            GL_TEXTURE_2D, BufTex[texture_index], 0);
}

void CoreTube::initTF()
{
  Tf.SetRGBAPoint(0.0, Color(0.0, 0.0, 150.0, 1.0));
  Tf.SetRGBAPoint(0.4, Color(230.0, 0.0, 0.0, 1.0));
  Tf.SetRGBAPoint(0.8, Color(230.0, 230.0, 0.0, 1.0));
  Tf.SetRGBAPoint(1.0, Color(255.0, 255.0, 255.0, 1.0));
  Tf.CreateTexture(256);
}

void CoreTube::initShaders()
{
  VertShader[0] = readShader(GL_VERTEX_SHADER, "shaders/color.vert");
  FragShader[0] = readShader(GL_FRAGMENT_SHADER, "shaders/color.frag");
  ProgShader[0] = glCreateProgramObjectARB();
  glAttachObjectARB(ProgShader[0], VertShader[0]);
  glAttachObjectARB(ProgShader[0], FragShader[0]);
  glLinkProgramARB(ProgShader[0]);
  glUseProgramObjectARB(ProgShader[0]);
  GLint texture_location = glGetUniformLocationARB(ProgShader[0], "tf");
  glUniform1iARB(texture_location, 0);
  glUseProgramObjectARB(0);

  VertShader[1] = readShader(GL_VERTEX_SHADER, "shaders/normal.vert");
  FragShader[1] = readShader(GL_FRAGMENT_SHADER, "shaders/normal.frag");
  ProgShader[1] = glCreateProgramObjectARB();
  glAttachObjectARB(ProgShader[1], VertShader[1]);
  glAttachObjectARB(ProgShader[1], FragShader[1]);
  glLinkProgramARB(ProgShader[1]);

  VertShader[2] = readShader(GL_VERTEX_SHADER, "shaders/scalar0.vert");
  FragShader[2] = readShader(GL_FRAGMENT_SHADER, "shaders/scalar0.frag");
  ProgShader[2] = glCreateProgramObjectARB();
  glAttachObjectARB(ProgShader[2], VertShader[2]);
  glAttachObjectARB(ProgShader[2], FragShader[2]);
  glLinkProgramARB(ProgShader[2]);

  VertShader[3] = readShader(GL_VERTEX_SHADER, "shaders/scalar1.vert");
  FragShader[3] = readShader(GL_FRAGMENT_SHADER, "shaders/scalar1.frag");
  ProgShader[3] = glCreateProgramObjectARB();
  glAttachObjectARB(ProgShader[3], VertShader[3]);
  glAttachObjectARB(ProgShader[3], FragShader[3]);
  glLinkProgramARB(ProgShader[3]);

  VertShader[4] = readShader(GL_VERTEX_SHADER, "shaders/scalar2.vert");
  FragShader[4] = readShader(GL_FRAGMENT_SHADER, "shaders/scalar2.frag");
  ProgShader[4] = glCreateProgramObjectARB();
  glAttachObjectARB(ProgShader[4], VertShader[4]);
  glAttachObjectARB(ProgShader[4], FragShader[4]);
  glLinkProgramARB(ProgShader[4]);

//  const GLubyte* opengl_version = glGetString(GL_VERSION);
//  std::cout << opengl_version << std::endl;
//  const GLubyte* shader_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
//  std::cout << shader_version << std::endl;
  for (int i = 0; i < 5; ++i)
  {
    GLint isVertCompiled, isFragCompiled;
    glGetShaderiv(VertShader[i], GL_COMPILE_STATUS, &isVertCompiled);
    glGetShaderiv(FragShader[i], GL_COMPILE_STATUS, &isFragCompiled);
    
    if (isVertCompiled == GL_FALSE)
    {
      std::cout << "VertShader[" << i << "]: ";
      printInfoLog(VertShader[i]);
    }
    if (isVertCompiled == GL_FALSE)
    {
      std::cout << "FragShader[" << i << "]: ";
      printInfoLog(FragShader[i]);
    }
  }
}

GLuint CoreTube::readShader(GLenum shader_type, const std::string& path)
{
  GLuint shader = glCreateShaderObjectARB(shader_type);

  std::ifstream fin(path.c_str(), std::ifstream::in);
  assert(fin.good());
  fin.seekg(0, std::ios::end);
  int length = fin.tellg();
  fin.seekg(0, std::ios::beg);
  char* buffer = new char [length];
  fin.read(buffer, length);
  fin.close();
  const char* const_buffer = buffer;

  glShaderSourceARB(shader, 1, &const_buffer, NULL);
  delete [] buffer;

  glCompileShaderARB(shader);
  return shader;
}

void CoreTube::printInfoLog(GLhandleARB obj)
{
  int infoLogLength = 0;
  int charsWritten = 0;
  char* infoLog;

  glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength);

  if (infoLogLength > 0)
  {
    infoLog = new char [infoLogLength];
    glGetInfoLogARB(obj, infoLogLength, &charsWritten, infoLog);
    std::cout << infoLog << "\n";
    delete [] infoLog;
  }
}

void CoreTube::setupCamera()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
//  glOrtho(-1, 1, -1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  CameraCore cam = Frames[CurCamIndex]->GetCamera();
  gluLookAt(cam.Position.x, cam.Position.y, cam.Position.z,
            cam.Focal.x, cam.Focal.y, cam.Focal.z,
            cam.ViewUp.x, cam.ViewUp.y, cam.ViewUp.z);

  calcDomain();

  double domain[6];
  Frames[CurCamIndex]->GetDataDomain(domain);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(domain[0], domain[1], domain[2], domain[3], domain[4], domain[5]);

  glMatrixMode(GL_MODELVIEW);
}

void CoreTube::setupLightEnv()
{
  GLfloat light_position[] = {LightPosition.x,
                              LightPosition.y,
                              LightPosition.z,
                              0.0};
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void CoreTube::drawTube(const Particle<>& p1, const Particle<>& p2)
{
  Vector pa(p2.x() - p1.x(), p2.y() - p1.y(), p2.z() - p1.z());
  double len = pa.len();
  pa.normalize();
  Vector za(0.0, 0.0, 1.0);
  za.normalize();
  Vector cr = pa ^ za; // cross product
  cr.normalize();
  double angle = acos(pa * za);

  glPushMatrix();

  glTranslatef(p1.x(), p1.y(), p1.z());
  glRotatef(-angle * 180.0 / M_PI, cr.x, cr.y, cr.z);

  int num_slices = 30;
  double angle_per_slice = 2 * M_PI / float(num_slices);

  glBegin(GL_QUADS);
  for (int i = 0; i < num_slices; ++i)
  {
    float x1 = radius() * cos(angle_per_slice * i);
    float y1 = radius() * sin(angle_per_slice * i);
    float x2 = radius() * cos(angle_per_slice * (i + 1));
    float y2 = radius() * sin(angle_per_slice * (i + 1));
//    Color color[2];
//    color[0] = Tf.GetColor(p1.pd);
//    color[1] = Tf.GetColor(p2.pd);
    Vector n;
    n = Vector(x1, y1, 0.0);
    n.normalize();

    for (unsigned int i = 0; i < p1.numScalars(); ++i)
      glMultiTexCoord1f(GL_TEXTURE0 + i, p1.scalar(i));
    glMultiTexCoord1f(GL_TEXTURE0 + p1.numScalars(), p1.id());
    glNormal3f(n.x, n.y, n.z); glVertex3f(x1, y1, 0.0);

    for (unsigned int i = 0; i < p2.numScalars(); ++i)
      glMultiTexCoord1f(GL_TEXTURE0 + i, p2.scalar(i));
    glMultiTexCoord1f(GL_TEXTURE0 + p2.numScalars(), p2.id());
    glNormal3f(n.x, n.y, n.z); glVertex3f(x1, y1, len);

    n = Vector(x2, y2, 0.0);
    n.normalize();

    for (unsigned int i = 0; i < p2.numScalars(); ++i)
      glMultiTexCoord1f(GL_TEXTURE0 + i, p2.scalar(i));
    glMultiTexCoord1f(GL_TEXTURE0 + p2.numScalars(), p2.id());
    glNormal3f(n.x, n.y, n.z); glVertex3f(x2, y2, len);

    for (unsigned int i = 0; i < p1.numScalars(); ++i)
      glMultiTexCoord1f(GL_TEXTURE0 + i, p1.scalar(i));
    glMultiTexCoord1f(GL_TEXTURE0 + p1.numScalars(), p1.id());
    glNormal3f(n.x, n.y, n.z); glVertex3f(x2, y2, 0.0);
  }
  glEnd();

  glPopMatrix();
}

void CoreTube::drawSphere(const Particle<>& p1, const Particle<>& p2)
{
  Vector pa(p2.x() - p1.x(), p2.y() - p1.y(), p2.z() - p1.z());
  double len = pa.len();
  pa.normalize();
  Vector za(0.0, 0.0, 1.0);
  za.normalize();
  Vector cr = pa ^ za; // cross product
  cr.normalize();
  double angle = acos(pa * za);

  glPushMatrix();

  for (unsigned int i = 0; i < p1.numScalars(); ++i)
    glMultiTexCoord1f(GL_TEXTURE0 + i, p1.scalar(i));
  glMultiTexCoord1f(GL_TEXTURE0 + p1.numScalars(), p1.id());

  glTranslatef(p1.x(), p1.y(), p1.z());
  glRotatef(-angle * 180.0 / M_PI, cr.x, cr.y, cr.z);

  GLUquadric* q = gluNewQuadric();
  gluQuadricNormals(q, GLU_SMOOTH);
  gluSphere(q, radius(), 30, 20);
  gluDeleteQuadric(q);

  glPopMatrix();
}

void CoreTube::snapshot()
{
  Frames[CurCamIndex]->SetSize(window_width(), window_height());
  Frames[CurCamIndex]->SetNumberOfScalarMaps(3);
  glEnable(GL_TEXTURE_2D);
  // depth
  glBindTexture(GL_TEXTURE_2D, BufTex[2 * 5 * CurCamIndex + 2 * 0 + 0]);
  float* depth = new float [window_area()];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
  Frames[CurCamIndex]->SetDepthMap(depth);
  delete [] depth;
  // color
  glBindTexture(GL_TEXTURE_2D, BufTex[2 * 5 * CurCamIndex + 2 * 0 + 1]);
  unsigned char* color = new unsigned char [window_area() * 4];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, color);
  Frames[CurCamIndex]->SetColorMap(color);
  delete [] color;
  // normal
  glBindTexture(GL_TEXTURE_2D, BufTex[2 * 5 * CurCamIndex + 2 * 1 + 1]);
  unsigned char* normal = new unsigned char [window_area() * 4];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_BYTE, normal);
  Frames[CurCamIndex]->SetNormalMap(normal);
  delete [] normal;
  // mf
  glBindTexture(GL_TEXTURE_2D, BufTex[2 * 5 * CurCamIndex + 2 * 2 + 1]);
  float* mf = new float [window_area()];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, mf);
  Frames[CurCamIndex]->SetScalarMap(0, reinterpret_cast<float *>(mf));
  delete [] mf;
  // temperature
  glBindTexture(GL_TEXTURE_2D, BufTex[2 * 5 * CurCamIndex + 2 * 3 + 1]);
  float* temperature = new float [window_area()];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, temperature);
  Frames[CurCamIndex]->SetScalarMap(1, temperature);
  delete [] temperature;
  // id
  glBindTexture(GL_TEXTURE_2D, BufTex[2 * 5 * CurCamIndex + 2 * 4 + 1]);
  float* id = new float [window_area()];
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, id);
  Frames[CurCamIndex]->SetScalarMap(2, id);
  delete [] id;
  // clean up
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

void CoreTube::mvMult(double matrix[16], double vec[4], double out[4]) const
{
  for (int i = 0; i < 4; ++i)
  {
    out[i] = 0.0;
    for (int j = 0; j < 4; ++j)
    {
      out[i] += matrix[i + 4 * j] * vec[j];
    }
  }
  for (int j = 0; j < 4; ++j)
  {
    out[j] /= out[3];
  }
}

void CoreTube::calcDomain()
{
  GLdouble modelMatrix[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
  double expand_extent[6];
  for (int i = 0; i < 3; ++i)
  {
    expand_extent[i * 2] = Extent[i * 2] - 2 * radius();
    expand_extent[i * 2 + 1] = Extent[i * 2 + 1] + 2 * radius();
  }
  double domain[6] = {FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX};

  for (int a = 0; a < 2; ++a)
    for (int b = 0; b < 2; ++b)
      for (int c = 0; c < 2; ++c)
      {
        double v[4] = {expand_extent[a], expand_extent[b + 2], expand_extent[c + 4], 1.0};
        double n[4];
        mvMult(modelMatrix, v, n);
        domain[0] = std::min(domain[0], n[0]);
        domain[1] = std::max(domain[1], n[0]);
        domain[2] = std::min(domain[2], n[1]);
        domain[3] = std::max(domain[3], n[1]);
        domain[4] = std::min(domain[4], n[2]);
        domain[5] = std::max(domain[5], n[2]);
      }
  double t = -domain[4];
  domain[4] = -domain[5];
  domain[5] = t;
/*
  std::cout << "Domain: ";
  for (int i = 0; i < 6; ++i)
  {
    std::cout << domain[i] << ", ";
  }
  std::cout << std::endl;
*/
  Frames[CurCamIndex]->SetDataDomain(domain);
}
