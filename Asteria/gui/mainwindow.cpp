#include "mainwindow.h"
#include "infra/asteriastate.h"
#include "gui/glmeteordrawer.h"
#include "gui/videodirectorymodel.h"


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
#include <QTreeView>


MainWindow::MainWindow(QWidget *parent, AsteriaState * state) : QMainWindow(parent), state(state)
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

    VideoDirectoryModel *model = new VideoDirectoryModel(state->videoDirPath);
    tree->setModel(model);
    tree->resizeColumnToContents(0);
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));

    acqThread = new AcquisitionThread(this, state);

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>)), drawer, SLOT (newFrame(std::shared_ptr<Image>)));

    // Connect new clip signal to tree viewer slot
    connect(acqThread, SIGNAL (acquiredClip(std::string)), model, SLOT (addNewClipByUtc(std::string)));

    acqThread->launch();

    show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    delete acqThread;
}
