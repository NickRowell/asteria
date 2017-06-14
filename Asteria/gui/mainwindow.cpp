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


MainWindow::MainWindow(QWidget *parent, AsteriaState * state) : QMainWindow(parent), state(state) {

    // Build GUI components
    live = new GLMeteorDrawer(this, this->state);
    replay = new GLMeteorDrawer(this, this->state);

    tabWidget = new QTabWidget;
    tabWidget->addTab(live, QString("Live"));
    tabWidget->addTab(replay, QString("Replay"));

    tree = new QTreeView(this);

    // Arrange layout
    central = new QWidget(this);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addStretch(1);
    mainLayout->addWidget(tabWidget);
    mainLayout->addStretch(1);
    central->setLayout(mainLayout);

    this->setCentralWidget(central);
}

void MainWindow::initAndShowGui() {

    // Initialisation to perform:
    // 1) Load the reference star catalogue
    // 2) Load the video directory contents into the viewer
    // 3) Create the acquisition thread and start it
    // ?) Load ephemeris file

    std::vector<ReferenceStar> refStarCatalogue = ReferenceStar::loadCatalogue(state->refStarCataloguePath);

    qInfo() << "Loaded " << refStarCatalogue.size() << " ReferenceStars!";

    VideoDirectoryModel *model = new VideoDirectoryModel(state->videoDirPath, tree);
    tree->setModel(model);
    tree->resizeColumnToContents(0);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Capture right-clicks in the tree view for displaying context menu
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    // Capture double-clicks in the tree view for replaying videos
    connect(tree, SIGNAL (doubleClicked(const QModelIndex)), this, SLOT(replayVideo(const QModelIndex)));

    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));

    acqThread = new AcquisitionThread(this, state);

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>)), live, SLOT (newFrame(std::shared_ptr<Image>)));
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>)), replay, SLOT (newFrame(std::shared_ptr<Image>)));

    // Connect new clip signal to tree viewer slot, so that new clips get added to the viewer
    connect(acqThread, SIGNAL (acquiredClip(std::string)), model, SLOT (addNewClipByUtc(std::string)));

    acqThread->launch();

    show();

}


void MainWindow::replayVideo(const QModelIndex &index) {
    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    qInfo() << "Double clicked on " << item->data(0);
    qInfo() << "TODO: implement playback, if this is a video clip";
}

void MainWindow::onCustomContextMenu(const QPoint &point) {
    QModelIndex index = tree->indexAt(point);
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        item->getContextMenu()->exec(tree->viewport()->mapToGlobal(point));
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    delete acqThread;
}
