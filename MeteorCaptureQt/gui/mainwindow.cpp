#include "mainwindow.h"
#include "infra/meteorcapturestate.h"

#include <QDebug>
#include <QCameraInfo>
#include <QCamera>


MainWindow::MainWindow(QWidget *parent, MeteorCaptureState * state) : QMainWindow(parent)
{
    this->state = state;
}

void MainWindow::slotInit() {
    qInfo() << "Launching camera " << state->qCameraInfo->description();

    // ...Initialise everything...
    camera = new QCamera(cameraInfo);


    show();
}
