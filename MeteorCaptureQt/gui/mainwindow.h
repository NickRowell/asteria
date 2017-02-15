#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    QCamera *camera;
    GLMeteorDrawer *drawer;
    void closeEvent(QCloseEvent *bar);

signals:
    void newFrameCaptured();

public slots:
    void slotInit();
};

#endif // MAINWINDOW_H
