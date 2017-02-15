#ifndef GLMETEORDRAWER_H
#define GLMETEORDRAWER_H

#include <QtGui>
#include <QOpenGLWidget>

class MeteorCaptureState;

class GLMeteorDrawer : public QOpenGLWidget {

    Q_OBJECT

public:
    GLMeteorDrawer(QWidget *parent, MeteorCaptureState * state = 0);
private:
    MeteorCaptureState * state;

protected:

    void initializeGL();

    void resizeGL(int w, int h);

    void paintGL();

public slots:
    void newFrame();
};

#endif // GLMETEORDRAWER_H
