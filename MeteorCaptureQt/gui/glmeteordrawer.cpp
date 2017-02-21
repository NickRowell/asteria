#include "gui/glmeteordrawer.h"
#include "infra/meteorcapturestate.h"

// Includes for open, close, lseek, ...
#include <fcntl.h>
#include <unistd.h>

GLMeteorDrawer::GLMeteorDrawer(QWidget *parent, MeteorCaptureState * state) : QOpenGLWidget(parent) {
    this->state = state;
}



void GLMeteorDrawer::newFrame() {
    // Copy the frame to device memory, trigger a redraw

    qInfo() << "Got new frame ";



}

void GLMeteorDrawer::initializeGL()
{
    // Create texture objects, load, compile & install shaders


    unsigned int i=0;
    // Set up the rendering context, define display lists etc.:
//        ...
//        glClearColor(0.0, 0.0, 0.0, 0.0);
//        glEnable(GL_DEPTH_TEST);
//        ...
}

void GLMeteorDrawer::resizeGL(int w, int h)
{
    unsigned int i=0;
    // setup viewport, projection etc.:
//        glViewport(0, 0, (GLint)w, (GLint)h);
//        ...
//        glFrustum(...);
//        ...
}

void GLMeteorDrawer::paintGL()
{
    // Draw the bound texture

    unsigned int i=0;
    // draw the scene:
//        ...
//        glRotatef(...);
//        glMaterialfv(...);
//        glBegin(GL_QUADS);
//        glVertex3f(...);
//        glVertex3f(...);
//        ...
//        glEnd();
//        ...
}


/**
 * Creates, compiles & links shaders. Returns the program ID by reference,
 * returns explicitly the link status of the program.
 * @param vShader
 * @param fShader
 * @param prog_id
 * @return
 */
int GLMeteorDrawer::installShaders(const GLchar * vShader, const GLchar * fShader, GLuint &prog_id)
{

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


int GLMeteorDrawer::readShaderSource (const char  *fileName, GLchar **shader)
{
  int size;
  /* Allocate memory to hold the source of our shaders. */
  size = shaderSize (fileName);

  if (size == -1)
    {
      printf ("Cannot determine size of the shader %s\n", fileName);
      return 0;
    }

  *shader = (GLchar*)malloc (size);

  /* Read the source code */
  if (!readShader (fileName, *shader, size))
    {
      printf ("Cannot read the file %s\n", fileName);
      return 0;
    }

  return 1;
}

int GLMeteorDrawer::readShader (const char  *fileName, char *shaderText, int size)
{
  /*
   * Reads a shader from the supplied file and returns the shader in the
   * arrays passed in. Returns 1 if successful, 0 if an error occurred.
   * The parameter size is an upper limit of the amount of bytes to read.
   * It is ok for it to be too big.
   */
  FILE *fh;
  char  name[100];
  int   count;

  strcpy (name, fileName);

  /* Open the file */
  fh = fopen (name, "r");
  if (!fh)
    return -1;

  /* Get the shader from a file. */
  fseek (fh, 0, SEEK_SET);
  count = (int) fread (shaderText, 1, size, fh);
  shaderText[count] = '\0';

  if (ferror (fh))
    count = 0;

  fclose (fh);
  return count;
}

int GLMeteorDrawer::shaderSize (const char *fileName)
{
  /*
   * Returns the size in bytes of the shader fileName.
   * If an error occurred, it returns -1.
   *
   * File name convention:
   *
   * <fileName>.vert
   * <fileName>.frag
   */
  int fd;
  char name[100];
  int count = -1;

  strcpy (name, fileName);

  /* Open the file, seek to the end to find its length */
  fd = open(name, O_RDONLY);
  if (fd != -1)
    {
      count = lseek (fd, 0, SEEK_END) + 1;
      ::close (fd);
    }

  return count;
}


int GLMeteorDrawer::printOpenGLError()
{
  /* Returns 1 if an OpenGL error occurred, 0 otherwise. */
  GLenum glErr;
  int    retCode = 0;

  glErr = glGetError ();
  while (glErr != GL_NO_ERROR)
    {
      printf ("glError: %s\n",glErr);
      retCode = 1;
      glErr = glGetError ();
    }
  return retCode;
}
