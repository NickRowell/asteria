#ifndef GLMETEORDRAWER_H
#define GLMETEORDRAWER_H

#include <QtGui>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

class MeteorCaptureState;

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

    explicit GLMeteorDrawer(QWidget *parent = 0, MeteorCaptureState *state = 0);
    ~GLMeteorDrawer();

    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setClearColor(const QColor &color);

private:
    MeteorCaptureState * state;

    // Handle to GL texture object used to store video frame
    GLuint VideoImageTexture;

    QOpenGLShaderProgram * program;
    QOpenGLBuffer vbo;

protected:

    void initializeGL() Q_DECL_OVERRIDE;

    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void paintGL() Q_DECL_OVERRIDE;

    int printOpenGLError();

public slots:

//    void newFrame(char *bufferStart);
    void newFrame(std::vector<char> bufferStart);
};

#endif // GLMETEORDRAWER_H
