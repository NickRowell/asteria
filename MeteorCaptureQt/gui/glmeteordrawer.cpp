#include "gui/glmeteordrawer.h"
#include "infra/meteorcapturestate.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

// Includes for open, close, lseek, ...
#include <fcntl.h>
#include <unistd.h>

GLMeteorDrawer::GLMeteorDrawer(QWidget *parent, MeteorCaptureState * state) : QOpenGLWidget(parent) {
    this->state = state;
    this->makeCurrent();
    this->initializeGL();
}

void GLMeteorDrawer::newFrame(char * bufferStart) {

//    // Copy the frame to device memory, trigger a redraw

//    qInfo() << "Got new frame ";

//    unsigned int width = 640;
//    unsigned int height = 480;

//    // TODO: map the pixel format in use by the camera to the appropriate GL type
//    GLenum format = GL_RGB;  // GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_RED_INTEGER

//    // TODO: set data type of the input pixels
//    GLenum type = GL_UNSIGNED_BYTE; // GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2,

//    // TODO: set the internal format used by the texture
//    GLint internalFormat = GL_R8; //

//    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, bufferStart);

//    // Post redraw
//    update();
}

void GLMeteorDrawer::initializeGL()
{
    qInfo() << "Initialising GL";

    initializeOpenGLFunctions();

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char * vsrc =
        "#version 330\n"
        // Vertex position
        "layout(location = 0) in vec3 position;\n"
        // Texture coordinates
        "layout(location = 1) in vec2 texCoords;\n"
        // Pass out the texture coordinates of the vertex
        "out vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "    texCoord   = texCoords;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char * fsrc =
        "#version 330\n"
        // Simple pass-through fragment shader used to render quad textures.
        "in vec2 texCoord;\n"           // <-- Texture coordinate of this fragment
        "out vec3 colourOut;\n"         // <-- Output from the fragment shader (pixel colour)
        "uniform sampler2D texture;\n"
        "void main()\n"
        "{\n"
        "    colourOut = texture2D(texture, texCoord).rgb;\n"
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




//    // Load, compile & install shaders, create texture object

//    // TODO: map the pixel format in use by the camera to the appropriate GL type
//    GLenum format = GL_RED;  // GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_RED_INTEGER

//    // TODO: set data type of the input pixels
//    GLenum type = GL_UNSIGNED_BYTE; // GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2,

//    // TODO: set the internal format used by the texture
//    GLint internalFormat = GL_R8; // GL_R16, GL_RGB8, GL_RGB32F

//    unsigned int width = 640;
//    unsigned int height = 480;

//    // Parse shader source (screen shaders)
//    GLchar *v_screen, *f_screen;
//    readShaderSource("v_screen.glsl", &v_screen);
//    readShaderSource("f_screen.glsl", &f_screen);

//    if(!installShaders(v_screen, f_screen, prog_id_screen))
//    {
//        printf("Screen shaders did not install correctly!\n");
//        return;
//    }
//    else
//    {
//        // Bind vertex attributes to specific indices (must be done before linking)
//        glBindAttribLocation(prog_id_screen, PositionAttributeIndex, "position");
//        glBindAttribLocation(prog_id_screen, TexCoordAttributeIndex, "texCoords");

//        // Shaders compiled correctly; now link into a program
//        if(!linkProgram(prog_id_screen))
//        {
//            printf("Screen program did not link correctly!\n");
//            return;
//        }
//        else
//        {
//            // Everything installed, compiled and linked: get uniform handles
//            uni_screen_tex = getUniLoc(prog_id_screen, "tex");
//        }
//    }

//    // Create & set up textures objects
//    glGenTextures(1, &VideoImageTexture);
//    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);
//    // Floating point rendering:
//    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

    qInfo() << "Finished initialising GL";
}

void GLMeteorDrawer::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
}

void GLMeteorDrawer::paintGL()
{
    unsigned int width = 640;
    unsigned int height = 480;

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Contains position and texture coordinates for each of four vertices, in
    // CLIP SPACE coordinates so that we don't need to apply any transformations
    // inside the vertex shader
    float quad[4][5]=
    {    // X       Y       Z      U      V
        {-1.0f, -1.0f, 0.0f,  0.0f,  0.0f},
        { 1.0f, -1.0f, 0.0f,  1.0f,  0.0f},
        {-1.0f,  1.0f, 0.0f,  0.0f,  1.0f},
        { 1.0f,  1.0f, 0.0f,  1.0f,  1.0f}
    };

    program->enableAttributeArray(PositionAttributeIndex);
    program->enableAttributeArray(TexCoordAttributeIndex);
    program->setAttributeArray(PositionAttributeIndex, GL_FLOAT, &quad[0][0], 3, 5*sizeof(float));
    program->setAttributeArray(TexCoordAttributeIndex, GL_FLOAT, &quad[0][3], 2, 5*sizeof(float));

    // Need to bind the texture
    texture->bind();

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    // Now draw both the scene radiance and camera images into the default
    // FrameBuffer onto a screen-aligned quad.
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);

//    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//        printf("Problem with default FrameBuffer!\n");
//    }

//    glUseProgram(prog_id_screen);

//    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
//    glClear(GL_COLOR_BUFFER_BIT);

//    glViewport(0, 0, width, height);

//    // Bind texture for rendering
//    glUniform1i(uni_screen_tex, 0);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, VideoImageTexture);

//    // Contains position and texture coordinates for each of four vertices, in
//    // CLIP SPACE coordinates so that we don't need to apply any transformations
//    // inside the vertex shader
//    float quad[4][5]=
//    {    // X       Y       Z      U      V
//        {-1.0f, -1.0f, 0.0f,  0.0f,  0.0f},
//        { 1.0f, -1.0f, 0.0f,  1.0f,  0.0f},
//        {-1.0f,  1.0f, 0.0f,  0.0f,  1.0f},
//        { 1.0f,  1.0f, 0.0f,  1.0f,  1.0f}
//    };

//    glEnableVertexAttribArray(PositionAttributeIndex);
//    glEnableVertexAttribArray(TexCoordAttributeIndex);

//    // Texture coordinates attribute
//    glVertexAttribPointer(PositionAttributeIndex, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), &quad[0][0]);
//    glVertexAttribPointer(TexCoordAttributeIndex, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), &quad[0][3]);
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//    glDisableVertexAttribArray(PositionAttributeIndex);
//    glDisableVertexAttribArray(TexCoordAttributeIndex);

//    // Show the image.
//    glFlush();
}

/**
 * Creates, compiles & links shaders. Returns the program ID by reference,
 * returns explicitly the link status of the program.
 * @param vShader
 * @param fShader
 * @param prog_id
 * @return
 */
int GLMeteorDrawer::installShaders(const GLchar * vShader, const GLchar * fShader, GLuint &prog_id) {

    GLuint vs, fs;           // Handles to objects
    GLint vComp, fComp;      // Status values

    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vs, 1, &vShader, NULL);
    glShaderSource(fs, 1, &fShader, NULL);

    glCompileShader(vs);

    printOpenGLError();

    glGetShaderiv(vs, GL_COMPILE_STATUS, &vComp);
    printShaderInfoLog(vs);

    glCompileShader(fs);
    printOpenGLError();
    glGetShaderiv(fs, GL_COMPILE_STATUS, &fComp);
    printShaderInfoLog(fs);

    if(!vComp || !fComp) return 0;

    prog_id = glCreateProgram();
    glAttachShader(prog_id, vs);
    glAttachShader(prog_id, fs);

    return 1;
}

int GLMeteorDrawer::linkProgram(GLuint &prog_id)
{
    glLinkProgram(prog_id);
    printOpenGLError();

    GLint linked;

    glGetProgramiv(prog_id, GL_LINK_STATUS, &linked);
    printProgramInfoLog(prog_id);

    return linked;
}

GLint GLMeteorDrawer::getUniLoc(GLuint program, const GLchar *name)
{
  GLint loc;

  loc = glGetUniformLocation (program, name);

  if (loc == -1)
    printf ("No such uniform named \"%s\"\n", name);

  printOpenGLError ();  // Check for OpenGL errors
  return loc;
}

void GLMeteorDrawer::printShaderInfoLog(GLuint shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);

    printOpenGLError();

    if (infologLen > 0)
    {
        infoLog = (GLchar*) malloc(infologLen);
        if (infoLog == NULL)
        {
            printf("ERROR: Could not allocate shader InfoLog buffer\n");
            exit(1);
        }
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
        printf("Shader InfoLog:\n%s",infoLog);
        free(infoLog);
    }
    printOpenGLError();
}

void GLMeteorDrawer::printProgramInfoLog(GLuint shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &infologLen);

    printOpenGLError();

    if (infologLen > 0)
    {
        infoLog = (GLchar*) malloc(infologLen);
        if (infoLog == NULL)
        {
            printf("ERROR: Could not allocate program InfoLog buffer\n");
            exit(1);
        }
        glGetProgramInfoLog(shader, infologLen, &charsWritten, infoLog);
        printf("Program InfoLog:\n%s",infoLog);
        free(infoLog);
    }
    printOpenGLError();
}


int GLMeteorDrawer::readShaderSource (const char  *fileName, GLchar **shader) {
    int size;

    /* Allocate memory to hold the source of our shaders. */
     size = shaderSize (fileName);

    if (size == -1) {
        printf ("Cannot determine size of the shader %s\n", fileName);
        return 0;
    }

    *shader = (GLchar*)malloc (size);

    /* Read the source code */
    if (!readShader (fileName, *shader, size)) {
        printf ("Cannot read the file %s\n", fileName);
        return 0;
    }
    return 1;
}

/*
 * Reads a shader from the supplied file and returns the shader in the
 * arrays passed in. Returns 1 if successful, 0 if an error occurred.
 * The parameter size is an upper limit of the amount of bytes to read.
 * It is ok for it to be too big.
 */
int GLMeteorDrawer::readShader (const char  *fileName, char *shaderText, int size) {

    FILE *fh;
    char  name[100];
    int   count;

    strcpy (name, fileName);

    /* Open the file */
    fh = fopen (name, "r");
    if (!fh) {
        return -1;
    }

    /* Get the shader from a file. */
    fseek (fh, 0, SEEK_SET);
    count = (int) fread (shaderText, 1, size, fh);
    shaderText[count] = '\0';

    if (ferror (fh)) {
        count = 0;
    }

    fclose (fh);
    return count;
}

/**
 * Returns the size in bytes of the shader fileName.
 * If an error occurred, it returns -1.
 *
 * File name convention:
 *
 * <fileName>.vert
 * <fileName>.frag
 * @brief GLMeteorDrawer::shaderSize
 * @param fileName
 * @return
 */
int GLMeteorDrawer::shaderSize (const char *fileName)
{
    int fd;
    char name[100];
    int count = -1;

    strcpy (name, fileName);

    // Open the file, seek to the end to find its length
    fd = open(name, O_RDONLY);
    if (fd != -1) {
         count = lseek (fd, 0, SEEK_END) + 1;
        ::close (fd);
    }

    return count;
}


int GLMeteorDrawer::printOpenGLError() {

    /* Returns 1 if an OpenGL error occurred, 0 otherwise. */
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError ();
    while (glErr != GL_NO_ERROR) {
        printf ("glError: %s\n",glErr);
        retCode = 1;
        glErr = glGetError ();
    }
    return retCode;
}
