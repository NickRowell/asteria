#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "infra/acquisitionthread.h"

class AsteriaState;
class GLMeteorDrawer;
class QCamera;
class QCloseEvent;

QT_FORWARD_DECLARE_CLASS(QTreeView)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, AsteriaState * state = 0);

private:

    AsteriaState * state;
    GLMeteorDrawer *drawer;
    QTreeView *tree;

    // Main container widget for all GUI elements
    QWidget * central;

    AcquisitionThread *acqThread;

    void closeEvent(QCloseEvent *bar);

signals:
    void newFrameCaptured(char * bufferStart);

public slots:
    void slotInit();
};

#endif // MAINWINDOW_H
