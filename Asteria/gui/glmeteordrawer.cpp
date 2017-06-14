#include "gui/glmeteordrawer.h"
#include "infra/asteriastate.h"
#include "util/timeutil.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

// Includes for open, close, lseek, ...
#include <fcntl.h>
#include <unistd.h>
#include <GL/glu.h>

#define PositionAttributeIndex 0
#define TexCoordAttributeIndex 1

GLMeteorDrawer::GLMeteorDrawer(QWidget *parent, AsteriaState *state)
    : QOpenGLWidget(parent), state(state), program(0) {

    font = new FTExtrudeFont("/usr/share/fonts/truetype/ubuntu-font-family/Ubuntu-M.ttf");

    if(font->Error()) {
        qInfo() << "Error loading font!";
        delete font;
    }
    else {
        font->Depth(0.0f);
        font->Outset(0, 1);
        font->FaceSize(16);
    }
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

    // For displaying the RGB annotated image:
//    unsigned char* a = &(image->annotatedImage[0]);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, a);

    // For displaying the RGBA annotated image with 32bit pixels:
    unsigned int* a = &(image->annotatedImage[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, a);

    timestamp = TimeUtil::convertToUtcString(image->epochTimeUs);
    fps = image->fps;
    droppedFrames = image->droppedFrames;
    totalFrames = image->totalFrames;

    // Post redraw
    update();
}

void GLMeteorDrawer::initializeGL() {

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
    program->release();

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
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // For displaying RGB annotated image from a texture, 32bit pixels:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

    // Set identity modelview and projection matrices. These are only relevant for rendering the
    // bitmapped timestamp into the image, for which the raster position is transformed and projected
    // into window coordinates using these.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0, state->width, 0, state->height );

    qInfo() << "Finished initialising GL";
}

void GLMeteorDrawer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void GLMeteorDrawer::paintGL() {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    program->enableAttributeArray(PositionAttributeIndex);
    program->enableAttributeArray(TexCoordAttributeIndex);
    program->setAttributeBuffer(PositionAttributeIndex, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    program->setAttributeBuffer(TexCoordAttributeIndex, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    // Bind the texture then render it onto screen-aligned quad
    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    program->disableAttributeArray(PositionAttributeIndex);
    program->disableAttributeArray(TexCoordAttributeIndex);
    program->release();

    // Timestamp using FTGL:
    if(font) {
        // Render inside
        glColor3f(0, 0.75, 0);
        font->Render(timestamp.c_str(), timestamp.size(), FTPoint(10, 10), FTPoint(1, 0), FTGL::RENDER_FRONT);
        // Render outline
        glColor3f(0, 0, 0);
        font->Render(timestamp.c_str(), timestamp.size(), FTPoint(10, 10), FTPoint(1, 0), FTGL::RENDER_SIDE);

        // Render FPS string
        char fpsArr [100];
        unsigned int length = sprintf (fpsArr, "FPS = %5.3f", fps);
        glColor3f(0, 0.75, 0);
        font->Render(fpsArr, length, FTPoint(10, 40), FTPoint(1, 0), FTGL::RENDER_FRONT);
        glColor3f(0, 0, 0);
        font->Render(fpsArr, length, FTPoint(10, 40), FTPoint(1, 0), FTGL::RENDER_SIDE);

        // Render dropped frames stats
        char dFpsArr [100];
        length = sprintf (dFpsArr, "Dropped frames = %5d / %5d", droppedFrames, totalFrames);
        glColor3f(0, 0.75, 0);
        font->Render(dFpsArr, length, FTPoint(10, 25), FTPoint(1, 0), FTGL::RENDER_FRONT);
        glColor3f(0, 0, 0);
        font->Render(dFpsArr, length, FTPoint(10, 25), FTPoint(1, 0), FTGL::RENDER_SIDE);
    }
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
