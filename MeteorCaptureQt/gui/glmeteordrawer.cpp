#include "gui/glmeteordrawer.h"
#include "infra/meteorcapturestate.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

// Includes for open, close, lseek, ...
#include <fcntl.h>
#include <unistd.h>

#define PositionAttributeIndex 0
#define TexCoordAttributeIndex 1

GLMeteorDrawer::GLMeteorDrawer(QWidget *parent, MeteorCaptureState *state)
    : QOpenGLWidget(parent), state(state), program(0) {
}

GLMeteorDrawer::~GLMeteorDrawer() {
    makeCurrent();
    vbo.destroy();
    delete program;
    doneCurrent();
}

QSize GLMeteorDrawer::sizeHint() const {
    return QSize(state->width, state->height);
}

void GLMeteorDrawer::newFrame(std::shared_ptr<Image> image) {

     glBindTexture(GL_TEXTURE_2D, VideoImageTexture);

     unsigned int width = state->width;
     unsigned int height = state->height;

     // For displaying the greyscale image:
//     unsigned char* a = &(image->rawImage[0]);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, a);

     unsigned char* a = &(image->annotatedImage[0]);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, a);



    // Post redraw
    update();
}

void GLMeteorDrawer::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char * vsrc =
        // Vertex position
        "attribute highp vec3 position;\n"
        // Texture coordinates
        "attribute mediump vec2 texCoords;\n"
        // Pass out the texture coordinates of the vertex
        "varying vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "    texCoord   = texCoords;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char * fsrc =
        // Simple pass-through fragment shader used to render quad textures.
        "varying vec2 texCoord;\n"           // <-- Texture coordinate of this fragment
        "uniform sampler2D texture;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texCoord);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);
    program->bindAttributeLocation("position", PositionAttributeIndex);
    program->bindAttributeLocation("texCoords", TexCoordAttributeIndex);
    program->link();
    program->bind();
    program->setUniformValue("texture", 0);

    // Contains position and texture coordinates for each of four vertices, in
    // CLIP SPACE coordinates so that we don't need to apply any transformations
    // inside the vertex shader
    static const float quad[4][5]=
    {    // X       Y       Z      U      V
        {-1.0f, -1.0f, 0.0f,  0.0f,  1.0f},
        { 1.0f, -1.0f, 0.0f,  1.0f,  1.0f},
        {-1.0f,  1.0f, 0.0f,  0.0f,  0.0f},
        { 1.0f,  1.0f, 0.0f,  1.0f,  0.0f}
    };

    QVector<GLfloat> vertData;
    for (int i=0; i < 4; ++i) {
        for ( int j = 0; j < 5; ++j) {
            vertData.append(quad[i][j]);
        }
    }
    vbo.create();
    vbo.bind();
    vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));

    //+++ Use underlying GL texture API +++//
    glGenTextures(1, &VideoImageTexture);
    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    unsigned int width = state->width;
    unsigned int height = state->height;

    // For displaying greyscale image from a texture:
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    // For displaying RGB annotated image from a texture:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    qInfo() << "Finished initialising GL";
}

void GLMeteorDrawer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void GLMeteorDrawer::paintGL()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program->enableAttributeArray(PositionAttributeIndex);
    program->enableAttributeArray(TexCoordAttributeIndex);
    program->setAttributeBuffer(PositionAttributeIndex, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    program->setAttributeBuffer(TexCoordAttributeIndex, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    // Bind the texture then render it onto screen-aligned quad
    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

int GLMeteorDrawer::printOpenGLError() {

    /* Returns 1 if an OpenGL error occurred, 0 otherwise. */
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError ();
    while (glErr != GL_NO_ERROR) {
        qInfo() << "glError: " << glErr;
        retCode = 1;
        glErr = glGetError ();
    }
    return retCode;
}
