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
 * QOpenGLWidget provides control over the main rendering pipeline.
 *
 * Originally this class additionally inherited from QOpenGLFunctions, but I couldn't get this
 * to work with multiple instances of the GLMeteorDrawer (required to show both the live and replay
 * footage). Removing the extra inheritance seemed to fix that without any further alterations.
 *
 * @brief The GLMeteorDrawer class
 */
class GLMeteorDrawer : public QOpenGLWidget {

    Q_OBJECT

public:

    explicit GLMeteorDrawer(QWidget *parent = 0, AsteriaState *state = 0, bool rgb = true);
    ~GLMeteorDrawer();

    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setClearColor(const QColor &color);

private:

    // The MeteorCaptureState object
    AsteriaState * state;

    // Handle to GL texture object used to store video frame
    GLuint VideoImageTexture;

    // Indicates RGB texture (true) or grey texture (false)
    bool rgb;

    // Shader program used to render textured quad
    QOpenGLShaderProgram * program;

    // Vertex buffer to store quad for rendering image as a texture
    QOpenGLBuffer vbo;

protected:

    void initializeGL() Q_DECL_OVERRIDE;

    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void paintGL() Q_DECL_OVERRIDE;

    int printOpenGLError();

public slots:

    void newFrame(std::shared_ptr<Image> image);
};

#endif // GLMETEORDRAWER_H
