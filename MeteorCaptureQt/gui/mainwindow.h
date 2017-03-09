#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "infra/acquisitionthread.h"

class MeteorCaptureState;
class GLMeteorDrawer;
class QCamera;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);

private:
    MeteorCaptureState * state;
    GLMeteorDrawer *drawer;

    AcquisitionThread *acqThread;

    void closeEvent(QCloseEvent *bar);

signals:
    void newFrameCaptured(char * bufferStart);

public slots:
    void slotInit();
};

#endif // MAINWINDOW_H
