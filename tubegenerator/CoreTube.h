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
#include "ConfigReader.h"

class Frame;

class CoreTube
{
public:
    CoreTube();
    ~CoreTube();

    void Initialize();
    void GenerateTubes(const std::vector<Particle<> >& particles1, const std::vector<Particle<> >& particles2);
    int GetCameraCount() const { return Frames.size(); }
    void Output();
    Frame* getFrame(const int index);

protected:
#ifdef USE_OSMESA
    OSMesaContext Ctx;
    GLubyte* ColorBuffer;
#else
    Display* dpy;
    GLXContext ctx;
#endif
    TransferFunction Tf;
    int CurCamIndex;
    std::vector<Frame*> Frames;
    GLuint DisplayListIndex;
    GLuint* Fbo;
    // 0 => depth :: 1 => color :: 2 => normal :: 3 => MF :: 4 => temperature :: 5 => ID
    GLuint* BufTex;
    GLhandleARB VertShader[5];
    GLhandleARB FragShader[5];
    GLhandleARB ProgShader[5];
    int BufferIndex;
    std::vector<int> times;

    ConfigReader& config() const {return ConfigReader::getInstance();}
    int window_width() const;
    int window_height() const;
    int window_area() const {return window_width() * window_height();}
    float radius() const;
    float max_particle_gap() const;
    void createDisplayList(const std::vector<Particle<> >& particles1, const std::vector<Particle<> >& particles2);
    void deleteDisplayList();
    void initGLContext();
    void initGLEnv();
    void initFrames();
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
    void setupClipPlane();
    void setupClipPlane(int index, Vector<> p, Vector<> n);
    void drawTube(const Particle<>& p1, const Particle<>& p2);
    void drawSphere(const Particle<>& p1, const Particle<>& p2);
    void snapshot();
    void mvMult(double matrix[16], double vec[4], double out[4]) const;
    void calcDomain();
};

#endif
