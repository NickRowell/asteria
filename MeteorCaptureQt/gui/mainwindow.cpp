#include "mainwindow.h"
#include "infra/meteorcapturestate.h"
#include "gui/glmeteordrawer.h"


#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QString>
#include <QCloseEvent>
#include <QGridLayout>
#include <QThread>
#include <QFileSystemModel>
#include <QTreeView>


MainWindow::MainWindow(QWidget *parent, MeteorCaptureState * state) : QMainWindow(parent), state(state)
{
    // Build GUI components
    drawer = new GLMeteorDrawer(this, this->state);
    tree = new QTreeView(this);

    // Arrange layout
    central = new QWidget(this);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addStretch(1);
    mainLayout->addWidget(drawer);
    mainLayout->addStretch(1);
    central->setLayout(mainLayout);

    this->setCentralWidget(central);

}

void MainWindow::slotInit() {

    // Initialisation to perform:
    // 1) Load the reference star catalogue
    // 2) Load the video directory contents into the viewer
    // 3) Create the acquisition thread and start it
    // ?) Load ephemeris file

    std::vector<ReferenceStar> refStarCatalogue = ReferenceStar::loadCatalogue(state->refStarCataloguePath);

    qInfo() << "Loaded " << refStarCatalogue.size() << " ReferenceStars!";

    QFileSystemModel *model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());
    model->setReadOnly(true);

    tree->setModel(model);
    tree->setRootIndex(model->index(QString::fromUtf8(state->videoDirPath.c_str())));
    tree->hideColumn(1);  // Hide 'Size'
    tree->hideColumn(2);  // Hide 'Type'
    tree->hideColumn(3);  // Hide 'Date Modified'
    tree->resizeColumnToContents(0);
    tree->setFixedWidth(250);
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));

    qInfo() << "Initialised video directory model at " << model->rootPath();

    acqWorker = new AcquisitionWorker(state);

    // Connect image acquisition signal to image display slot
    connect(acqWorker, SIGNAL (acquiredImage(std::shared_ptr<Image>)), drawer, SLOT (newFrame(std::shared_ptr<Image>)));

    acqThread = new QThread;

    acqWorker->moveToThread(acqThread);

    // ACQUISITION STARTUP:
    connect(acqThread, SIGNAL(started()), acqWorker, SLOT(launch()));

    // ACQUISITION SHUTDOWN:

    // Need to respond to user closing the window, and to termination from the command line...
    connect(acqWorker, SIGNAL(finished()), acqThread, SLOT(quit()));

    // ACQUISITION INITIALISATION:
    // ACQUISITION PAUSE:
    // ACQUISITION RESUME:

    // HOUSEKEEPING:
    // Delete the thread only after it's finished signal has been emitted
    connect(acqThread, SIGNAL(finished()), acqThread, SLOT(deleteLater()));

    acqThread->start();

    show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qInfo() << "Close event detected";
    acqWorker->shutdownLater();
}
