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


MainWindow::MainWindow(QWidget *parent, MeteorCaptureState * state) : QMainWindow(parent), state(state)
{
    drawer = new GLMeteorDrawer(this, this->state);
    this->setCentralWidget(drawer);
}

void MainWindow::slotInit() {

    acqThread = new AcquisitionThread(this, state);

    // Connect image acquisition signal to image display slot
//    connect(acqThread, SIGNAL (acquiredImage(char *)), drawer, SLOT (newFrame(char *)));
    connect(acqThread, SIGNAL (acquiredImage(std::vector<char>)), drawer, SLOT (newFrame(std::vector<char>)));

    acqThread->launch();

    show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    delete acqThread;
}
