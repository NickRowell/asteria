#include "gui/glmeteordrawer.h"
#include "infra/meteorcapturestate.h"

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

