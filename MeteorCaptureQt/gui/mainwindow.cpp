#include "mainwindow.h"
#include "infra/meteorcapturestate.h"
#include "gui/glmeteordrawer.h"


#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>


MainWindow::MainWindow(QWidget *parent, MeteorCaptureState * state) : QMainWindow(parent)
{
    this->state = state;

    drawer = new GLMeteorDrawer(this, state);

    this->setCentralWidget(drawer);

    show();
}

void MainWindow::slotInit() {

    show();
    acqThread = new AcquisitionThread(this, state);

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(char *)), drawer, SLOT (newFrame(char *)));

    acqThread->launch();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    delete acqThread;
}
