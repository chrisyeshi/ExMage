#ifndef VIEWER_H
#define VIEWER_H

#include <string>
#include <QGLWidget>
#include <QtOpenGL>
#include <QColor>
#include "APNGReader.h"
#include "TF.h"

class Viewer : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT
public:
    explicit Viewer(QWidget *parent = 0);

    bool open(const std::string& filename);
    std::string getInfo() const;
    
signals:
    void infoChanged();
    void zoomSignal(int level);
    
public slots:
    void tfChanged(mslib::TF& tf);
    void zoomChanged(int level);
    void cutChanged(int distance);

protected:
    //
    //
    // Inherited from QGLWidget
    //
    //
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

    //
    //
    // My functions
    //
    //
    void initTF();
    void initShaders();
    void reset();
    void updateOrtho();
    void updateCamera();
    void updateVBO();
    void scaleDomain();
    float calcPointSize() const;
    float minYRangeDomain() const;
    float maxYRangeDomain() const;
    float minDomainResRatio() const;
    float maxPointSize() const;

private:
    //
    //
    // Static const members
    //
    //
    const static QColor bgColor;

    //
    //
    // Private members
    //
    //
    APNGReader mpng;
    unsigned int nVerts;
    CameraCore camera;
    std::vector<float> domain;
    GLuint vbo;
    GLuint tfTex;
    QGLShaderProgram shader;
    QPoint mousePos;

    //
    //
    // Private helper classes
    //
    //
    class Vertex
    {
    public:
        Vertex(GLfloat x, GLfloat y, GLfloat z,
               GLfloat nx, GLfloat ny, GLfloat nz, GLfloat s = 0.f);
        GLfloat x, y, z, nx, ny, nz, s;
        bool operator<(const Vertex& right) const;
    };
};

#endif // VIEWER_H
