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

void GLMeteorDrawer::newFrame(std::shared_ptr<Image> image, bool renderTopField, bool renderBottomField) {

    unsigned int width = state->width;
    unsigned int height = state->height;

    // Render the full frame
    if(renderTopField && renderBottomField) {
        // For displaying the greyscale image:
        unsigned char* acquired = &(image->rawImage[0]);
        glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, acquired);
        renderVideoImageTexture = true;
    }
    else {
        renderVideoImageTexture = false;
    }
    // Render one or the other field
    if(renderTopField != renderBottomField) {

        isTopField = renderTopField;

        // Skip every second row in order to load just the odd or even pixel rows to the texture. We apply a small
        // vertical shift correction to the texture coordinates at the rendering stage to compensate for the slight
        // displacement in the displayed image that would otherwise occur.
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 2*width);
        if(renderBottomField) {
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, width);
        }

        unsigned char* acquired = &(image->rawImage[0]);
        glBindTexture(GL_TEXTURE_2D, VideoFieldTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, acquired);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        renderVideoFieldTexture = true;
    }
    else {
        renderVideoFieldTexture = false;
    }

    // For displaying the RGBA annotated image with 32bit pixels:
    if(!image->annotatedImage.empty()) {
        glBindTexture(GL_TEXTURE_2D, OverlayImageTexture);
        unsigned int* annotated = &(image->annotatedImage[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, annotated);
        renderOverlayImageTexture = true;
    }
    else {
        renderOverlayImageTexture = false;
    }

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
        "uniform float voffset;\n"           // <-- Offset applied to u texture coordinate to align top and bottom fields when displaying interlaced scan images
        "void main()\n"
        "{\n"
        "    vec2 texUpdated = vec2(texCoord.x, texCoord.y + voffset);\n"
        "    gl_FragColor = texture2D(texture, texUpdated);\n"
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
    program->setUniformValue("voffset", 0.0f);
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

    unsigned int width = state->width;
    unsigned int height = state->height;

    // Create textures using underlying GL API
    GLuint texHandles[3];
    glGenTextures(3, texHandles);
    VideoImageTexture = texHandles[0];
    VideoFieldTexture = texHandles[1];
    OverlayImageTexture = texHandles[2];

    // Create VideoImageTexture
    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // For displaying greyscale image from a texture:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    // Create VideoFieldTexture
    glBindTexture(GL_TEXTURE_2D, VideoFieldTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // For displaying greyscale image from a texture:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

    // Create OverlayImageTexture
    glBindTexture(GL_TEXTURE_2D, OverlayImageTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // For displaying RGBA annotated image from a texture, 32bit pixels:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, NULL);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set identity modelview and projection matrices. These are only relevant for rendering the
    // bitmapped timestamp into the image, for which the raster position is transformed and projected
    // into window coordinates using these.
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0, width, 0, height);
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

    // For each texture to be rendered, bind the texture then render it onto screen-aligned quad

    if(renderVideoImageTexture) {
        glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    if(renderVideoFieldTexture) {

        // Apply vertical shift to compensate for top/bottom field displacement.
        // Shift is quarter of one texture pixel, normalised to texture coordinates.
        float shift = 1.0 / (2.0 * state->height);

        if(isTopField) {
            program->setUniformValue("voffset", shift);
        }
        else {
            program->setUniformValue("voffset", -shift);
        }

        glBindTexture(GL_TEXTURE_2D, VideoFieldTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        program->setUniformValue("voffset", 0.0f);
    }

    if(renderOverlayImageTexture) {
        glBindTexture(GL_TEXTURE_2D, OverlayImageTexture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    program->disableAttributeArray(PositionAttributeIndex);
    program->disableAttributeArray(TexCoordAttributeIndex);
    program->release();

    glFlush();
}

int GLMeteorDrawer::printOpenGLError() {

    // Returns 1 if an OpenGL error occurred, 0 otherwise
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError ();
    while (glErr != GL_NO_ERROR) {
        fprintf(stderr, "glError: %d\n", glErr);
        retCode = 1;
        glErr = glGetError ();
    }
    return retCode;
}
