#include "viewer.h"
#include <sstream>
#include <GL/glu.h>
#include <QPoint>
#include "Frame.h"

//
//
// Static Const Members Initialization
//
//

const QColor Viewer::bgColor = QColor::fromHsv(0, 0, 50);

//
//
// Constructor / Destructor
//
//

Viewer::Viewer(QWidget *parent) :
    QGLWidget(parent), domain(6), vbo(0), tfTex(0)
{
}

//
//
// Public Methods
//
//

bool Viewer::open(const std::string& filename)
{
    reset();
    mpng.setFileName(filename);
    if (!mpng.read())
    {
        // TODO: handle error
        return false;
    }
    // camera
    domain = mpng.getGlobalDomain();
    scaleDomain();
    updateVBO();
    updateOrtho();
    camera = mpng.getCamera();
    updateCamera();
    emit infoChanged();
    updateGL();
    return true;
}

std::string Viewer::getInfo() const
{
    std::stringstream ss;
    ss << "No. Verts: " << nVerts << std::endl;
    ss << "CamPos: " << camera.Position << std::endl;
    ss << "CamFoc: " << camera.Focal << std::endl;
    ss << "CamVup: " << camera.ViewUp << std::endl;
    ss << "Domain: [" << domain[0] << ", " << domain[1] << ", " << domain[2] << ", " << domain[3] << ", " << domain[4] << ", " << domain[5] << "]" << std::endl;
    return ss.str();
}

//
//
// Public Slots
//
//

void Viewer::tfChanged(mslib::TF &tf)
{
    // check if the transfer function is initialized
    if (tfTex == 0)
        return;
    // update texture
    glBindTexture(GL_TEXTURE_1D, tfTex);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, tf.resolution(), 0, GL_RGBA, GL_FLOAT, tf.colorMap());
    glBindTexture(GL_TEXTURE_1D, 0);
    updateGL();
}

void Viewer::zoomChanged(int level)
{
    float oriYRange = mpng.getGlobalDomain()[3] - mpng.getGlobalDomain()[2];
    float yRange = (float(level) / 999.f) * (minYRangeDomain() - oriYRange) + oriYRange;
    float yMin = (domain[3] + domain[2]) * 0.5 - yRange * 0.5;
    float yMax = (domain[3] + domain[2]) * 0.5 + yRange * 0.5;
    domain[2] = yMin; domain[3] = yMax;
    scaleDomain();
    updateOrtho();
    emit infoChanged();
    updateGL();
}

void Viewer::cutChanged(int distance)
{
    float zMin = mpng.getGlobalDomain()[4];
    float zMax = mpng.getGlobalDomain()[5];
    float z = (float(distance) / 999.f) * (zMax - zMin) + zMin;
    domain[4] = z;
    updateOrtho();
    updateGL();
}

//
//
// Protected Methods: Inherited from QGLWidget
//
//

void Viewer::initializeGL()
{
    initializeGLFunctions();
    qglClearColor(bgColor);
    glGenBuffers(1, &vbo);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POINT_SPRITE);
    initTF();
    initShaders();
}

void Viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    qglColor(QColor::fromHsv(180, 0, 200));

    glPointSize(calcPointSize());

    // opengl draw routines
    // enable shaders...
    shader.bind();
    // enable textures...
    glEnable(GL_TEXTURE_1D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, tfTex);
    // enable client states...
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    // vertex buffer pointers
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glNormalPointer(GL_FLOAT, 0, (const GLvoid*)(nVerts * 3 * sizeof(GLfloat)));
    glTexCoordPointer(1, GL_FLOAT, 0, (const GLvoid*)(nVerts * 6 * sizeof(GLfloat)));
    // DRAW!!!
    glDrawArrays(GL_POINTS, 0, nVerts);
    // clean up client states...
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // clean up textures...
    glBindTexture(GL_TEXTURE_1D, 0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_1D);
    // clean up shaders...
    shader.release();
}

void Viewer::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    scaleDomain();
    updateOrtho();
    emit infoChanged();
}

void Viewer::mousePressEvent(QMouseEvent *event)
{
    mousePos = event->pos();
}

void Viewer::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    { // pan
        GLdouble mMatrix[16], pMatrix[16];
        GLint viewport[4];
        glGetDoublev(GL_MODELVIEW_MATRIX, mMatrix);
        glGetDoublev(GL_PROJECTION_MATRIX, pMatrix);
        glGetIntegerv(GL_VIEWPORT, viewport);
        Point prev, curr;
        gluUnProject(mousePos.x(), height() - mousePos.y(), 1.0, mMatrix, pMatrix, viewport, &prev.x, &prev.y, &prev.z);
        gluUnProject(event->x(), height() - event->y(), 1.0, mMatrix, pMatrix, viewport, &curr.x, &curr.y, &curr.z);
        camera.pan(curr - prev);
        updateCamera();
        emit infoChanged();
        updateGL();
    }
    mousePos = event->pos();
}

void Viewer::mouseReleaseEvent(QMouseEvent *event)
{
    mousePos = event->pos();
}

void Viewer::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    float ratio = 1.0 - float(numSteps) / 10.0;
    float yRange = domain[3] - domain[2];
    yRange *= ratio;
    yRange = yRange < minYRangeDomain() ? minYRangeDomain() : yRange;
    yRange = yRange > maxYRangeDomain() ? maxYRangeDomain() : yRange;
    float yMin = (domain[3] + domain[2]) * 0.5 - yRange * 0.5;
    float yMax = (domain[3] + domain[2]) * 0.5 + yRange * 0.5;
    domain[2] = yMin; domain[3] = yMax;
    scaleDomain();
    updateOrtho();
    emit infoChanged();
    updateGL();
    // signal the zoom slider
    int level = (yRange - maxYRangeDomain()) / (minYRangeDomain() - maxYRangeDomain()) * 999.f + 0.5f;
    emit zoomSignal(level);
}

//
//
// Protected Methods: My functions
//
//

void Viewer::initTF()
{
    mslib::TF tf;
    glGenTextures(1, &tfTex);
    glBindTexture(GL_TEXTURE_1D, tfTex);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, tf.resolution(), 0, GL_RGBA, GL_FLOAT, tf.colorMap());
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_1D, 0);
}

void Viewer::initShaders()
{
    shader.removeAllShaders();
    shader.addShaderFromSourceFile(QGLShader::Vertex, "shaders/pointcloud.vert");
    shader.addShaderFromSourceFile(QGLShader::Fragment, "shaders/pointcloud.frag");
    shader.link();
    shader.bind();
    shader.setUniformValue("tf", 0);
    shader.release();
}

void Viewer::reset()
{
//    glDeleteBuffers(1, &vbo); vbo = 0;
//    glDeleteTextures(1, &tfTex); tfTex = 0;
}

void Viewer::updateOrtho()
{
    makeCurrent();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(domain[0], domain[1], domain[2], domain[3], domain[4], domain[5]);

    glMatrixMode(GL_MODELVIEW);
}

void Viewer::updateCamera()
{
    makeCurrent();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera.Position.x, camera.Position.y, camera.Position.z,
              camera.Focal.x,    camera.Focal.y,    camera.Focal.z,
              camera.ViewUp.x,   camera.ViewUp.y,   camera.ViewUp.z);
}

void Viewer::updateVBO()
{
    makeCurrent();
    std::vector<Vertex> sortVerts;
    std::vector<Frame*> frames = mpng.getFrames();
    for (unsigned int i = 0; i < frames.size(); ++i)
    {
        Frame* f = frames[i];
        int size[2];
        f->GetSize(size);
        for (int x = 0; x < size[0]; ++x)
        for (int y = 0; y < size[1]; ++y)
        {
            int pixel = x + y * size[0];
            float* depthMap = f->GetDepthMap();
            float* normalMap = f->GetNormalMap();
            float* scalarMap = f->GetScalarMap(0);
            GLfloat z = depthMap[pixel];
            if (z > 0.9999 || z < 0.0001)
                continue;
            std::vector<float> oriDomain = mpng.getGlobalDomain();
            GLfloat zz = z * (oriDomain[5] - oriDomain[4]) + oriDomain[4];
            GLfloat nxx = GLfloat(x) / size[0];
            GLfloat xx = nxx * (oriDomain[1] - oriDomain[0]) + oriDomain[0];
            GLfloat nyy = GLfloat(y) / size[1];
            GLfloat yy = nyy * (oriDomain[3] - oriDomain[2]) + oriDomain[2];
            CameraCore cam = mpng.getCamera();
            Vector view = cam.Focal - cam.Position;
            view.normalize();
            Vector t = cam.ViewUp;
            t.normalize();
            Vector s = view ^ t;
            s.normalize();
            Vector u = t ^ s;
            u.normalize();
            Point p = double(xx) * s + double(yy) * t + double(zz) * u + cam.Position;
            GLfloat nx = normalMap[3 * pixel + 0];
            GLfloat ny = normalMap[3 * pixel + 1];
            GLfloat nz = normalMap[3 * pixel + 2];
            GLfloat scalar = scalarMap[pixel];
            sortVerts.push_back(Vertex(p.x, p.y, p.z, nx, ny, nz, scalar));
        }
    }
    std::sort(sortVerts.begin(), sortVerts.end());
    this->nVerts = sortVerts.size();
    std::vector<GLfloat> vertices(sortVerts.size() * 3);
    std::vector<GLfloat> normals(sortVerts.size() * 3);
    std::vector<GLfloat> scalars(sortVerts.size());
    for (unsigned int i = 0; i < sortVerts.size(); ++i)
    {
        vertices[3 * i + 0] = sortVerts[i].x;
        vertices[3 * i + 1] = sortVerts[i].y;
        vertices[3 * i + 2] = sortVerts[i].z;
        normals[3 * i + 0] = sortVerts[i].nx;
        normals[3 * i + 1] = sortVerts[i].ny;
        normals[3 * i + 2] = sortVerts[i].nz;
        scalars[i] = sortVerts[i].s;
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, nVerts * 7 * sizeof(GLfloat), 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nVerts * 3 * sizeof(GLfloat), &vertices[0]);
    glBufferSubData(GL_ARRAY_BUFFER, nVerts * 3 * sizeof(GLfloat), nVerts * 3 * sizeof(GLfloat), &normals[0]);
    glBufferSubData(GL_ARRAY_BUFFER, nVerts * 6 * sizeof(GLfloat), nVerts * sizeof(GLfloat), &scalars[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Viewer::scaleDomain()
{
    std::vector<int> oriResolution = mpng.getResolution();
    float oriResRatio = float(oriResolution[0]) / oriResolution[1];
    std::vector<float> oriDomain = mpng.getGlobalDomain();
    float oriDomainRatio = (oriDomain[1] - oriDomain[0]) / (oriDomain[3] - oriDomain[2]);
    float resRatio = float(width()) / height();
    float domainRatio = resRatio * oriDomainRatio / oriResRatio;
    float xRange = (domain[3] - domain[2]) * domainRatio;
    float xMin = (domain[1] + domain[0]) * 0.5 - xRange * 0.5;
    float xMax = (domain[1] + domain[0]) * 0.5 + xRange * 0.5;
    domain[0] = xMin;
    domain[1] = xMax;
}

float Viewer::calcPointSize() const
{
    float oriYDomain = mpng.getGlobalDomain()[3] - mpng.getGlobalDomain()[2];
    float oriYResolution = mpng.getResolution()[1];
    float oriRatio = oriYDomain / oriYResolution;
    float yDomain = domain[3] - domain[2];
    float yResolution = height();
    float ratio = yDomain / yResolution;
    return oriRatio / ratio + .5f;
}

float Viewer::minYRangeDomain() const
{
    return minDomainResRatio() * height();
}

float Viewer::maxYRangeDomain() const
{
    return mpng.getGlobalDomain()[3] - mpng.getGlobalDomain()[2];
}

float Viewer::minDomainResRatio() const
{
    float oriYDomain = mpng.getGlobalDomain()[3] - mpng.getGlobalDomain()[2];
    float oriYResolution = mpng.getResolution()[1];
    float oriRatio = oriYDomain / oriYResolution;
    return oriRatio / maxPointSize();
}

float Viewer::maxPointSize() const
{
    float range[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, range);
    return range[1];
}

//
//
// Private Methods
//
//

//
//
// Private Helper Structs
//
//

Viewer::Vertex::Vertex(GLfloat x, GLfloat y, GLfloat z,
                       GLfloat nx, GLfloat ny, GLfloat nz, GLfloat s)
    : x(x), y(y), z(z), nx(nx), ny(ny), nz(nz), s(s)
{
}

bool Viewer::Vertex::operator<(const Vertex& right) const
{
    return this->z < right.z;
}
