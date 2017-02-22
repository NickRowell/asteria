#ifndef GLMETEORDRAWER_H
#define GLMETEORDRAWER_H

#include <QtGui>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class MeteorCaptureState;

/**
 * QOpenGLWidget provides control over the main rendering pipeline,
 * QOpenGLFunctions provides functions to set up the rendering context.
 *
 * @brief The GLMeteorDrawer class
 */
class GLMeteorDrawer : public QOpenGLWidget, protected QOpenGLFunctions {

    Q_OBJECT

public:
    GLMeteorDrawer(QWidget *parent, MeteorCaptureState * state = 0);
private:
    MeteorCaptureState * state;

    // Handle to GL program used to draw textures onto screen
    GLuint prog_id_screen;
    // Handle to GL texture object used to store cideo frame
    GLuint VideoImageTexture;
    // Handle to location in device memory of the texture memory
    GLint uni_screen_tex;

    // Vertex attribute indices used in my shaders
    static const GLuint PositionAttributeIndex = 0;
    static const GLuint TexCoordAttributeIndex = 1;
    static const GLuint NormalAttributeIndex   = 2;
    static const GLuint ColourAttributeIndex   = 3;

protected:

    void initializeGL();

    void resizeGL(int w, int h);

    void paintGL();

    // Function used to load, compile and install shaders
    int installShaders(const GLchar *, const GLchar *, GLuint &);
    // Function used to link program
    int linkProgram(GLuint &);

    int printOpenGLError();
    void printShaderInfoLog(GLuint);
    void printProgramInfoLog(GLuint);
    GLint getUniLoc (GLuint, const GLchar *);

    // Shader source parsing functions
    int readShaderSource (const char  *, GLchar **);
    int readShader (const char *, char *, int);
    int shaderSize (const char *);

public slots:
    void newFrame(char *bufferStart);
};

#endif // GLMETEORDRAWER_H
