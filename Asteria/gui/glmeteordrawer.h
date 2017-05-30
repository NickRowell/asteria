#ifndef GLMETEORDRAWER_H
#define GLMETEORDRAWER_H

#include "infra/image.h"

#include <memory>           // shared_ptr
#include <FTGL/ftgl.h>
#include <QtGui>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

class AsteriaState;

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)

/**
 * QOpenGLWidget provides control over the main rendering pipeline,
 * QOpenGLFunctions provides functions to set up the rendering context.
 *
 * @brief The GLMeteorDrawer class
 */
class GLMeteorDrawer : public QOpenGLWidget, protected QOpenGLFunctions {

    Q_OBJECT

public:

    explicit GLMeteorDrawer(QWidget *parent = 0, AsteriaState *state = 0);
    ~GLMeteorDrawer();

    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setClearColor(const QColor &color);

private:

    // The MeteorCaptureState object
    AsteriaState * state;

    // Handle to GL texture object used to store video frame
    GLuint VideoImageTexture;

    // Shader program used to render textured quad
    QOpenGLShaderProgram * program;

    // Vertex buffer to store quad for rendering image as a texture
    QOpenGLBuffer vbo;

    // Timestamp string of current image
    std::string timestamp;

    // Current frames-per-second
    double fps;
    // Dropped frame statistics
    unsigned int droppedFrames;
    unsigned int totalFrames;

    // Font used to render timestamp
    FTExtrudeFont * font;

protected:

    void initializeGL() Q_DECL_OVERRIDE;

    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void paintGL() Q_DECL_OVERRIDE;

    int printOpenGLError();

public slots:

//    void newFrame(char *bufferStart);
    void newFrame(std::shared_ptr<Image> image);
};

#endif // GLMETEORDRAWER_H
