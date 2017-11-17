#ifndef GLMETEORDRAWER_H
#define GLMETEORDRAWER_H

#include "infra/imageuc.h"

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
 * @brief The GLMeteorDrawer class.
 *
 * QOpenGLWidget provides control over the main rendering pipeline.
 *
 * Originally this class additionally inherited from QOpenGLFunctions, but I couldn't get this
 * to work with multiple instances of the GLMeteorDrawer. Removing the extra inheritance seemed
 * to fix that without any further alterations.
 */
class GLMeteorDrawer : public QOpenGLWidget {

    Q_OBJECT

public:

    explicit GLMeteorDrawer(QWidget *parent, const unsigned int &width, const unsigned int &height);
    ~GLMeteorDrawer();

    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setClearColor(const QColor &color);

private:

    /**
     * @brief Width of the images being displayed [pixels]
     */
    unsigned int width;

    /**
     * @brief Height of the images being displayed [pixels]
     */
    unsigned int height;

    /**
     * @brief Texture handle for (width*height) acquired image display.
     */
    GLuint VideoImageTexture;

    /**
     * @brief Texture handle for (width * height/2) acquired image when interlaced scan
     * mode was in use and we're doing de-interlaced stepping.
     */
    GLuint VideoFieldTexture;

    /**
     * @brief Texture handle for (width*height) annotated image display.
     */
    GLuint OverlayImageTexture;

    /**
     * @brief Flag used to indicate whether the VideoImageTexture texture should be rendered.
     */
    bool renderVideoImageTexture;

    /**
     * @brief Flag used to indicate whether the VideoFieldTexture texture should be rendered.
     */
    bool renderVideoFieldTexture;

    /**
     * @brief Flag used to indicate whether the VideoOverlayImageTexture texture should be rendered.
     */
    bool renderOverlayImageTexture;

    /**
     * @brief Flag indicating if the current VideoFieldTexture is for the top or bottom field.
     */
    bool isTopField;

    /**
     * @brief Shader program used to render textured quad
     */
    QOpenGLShaderProgram * program;

    /**
     * @brief Vertex buffer to store quad for rendering image as a texture
     */
    QOpenGLBuffer vbo;

protected:

    void initializeGL() Q_DECL_OVERRIDE;

    void resizeGL(int w, int h) Q_DECL_OVERRIDE;

    void paintGL() Q_DECL_OVERRIDE;

    int printOpenGLError();

public slots:

    void newFrame(std::shared_ptr<Imageuc> image, bool renderOverlay, bool renderTopField, bool renderBottomField);
};

#endif // GLMETEORDRAWER_H
