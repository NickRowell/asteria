#include "mainwindow.h"
#include "infra/asteriastate.h"
#include "gui/glmeteordrawer.h"
#include "gui/videodirectorymodel.h"
#include "util/timeutil.h"

#include <fstream>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <regex>

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

    QString title = item->data(0).toString();
    QString path = item->data(1).toString();

    // TODO: detect if this is a video clip

    qInfo() << "Double clicked on " << title << "(" << path << ")";
    qInfo() << "TODO: implement playback, if this is a video clip";

    // Regex suitable for identifying images with filenames e.g. 2017-06-14T19:41:09.282Z
    const std::regex utcRegex = TimeUtil::getUtcRegex();
    const std::regex peakHoldRegex = std::regex("peakhold");

    // Load all the images found here...
    DIR *dir;
    if ((dir = opendir (path.toStdString().c_str())) == NULL) {
        // Couldn't open the directory!
        return;
    }

    Image peakHold;
//    std::shared_ptr<Image> peakHold;// = make_shared<Image>(state->width, state->height);

    std::vector<Image> sequence;

    // Loop over the contents of the directory
    struct dirent *child;
    while ((child = readdir (dir)) != NULL) {

        // Skip the . and .. directories
        if(strcmp(child->d_name,".") == 0 || strcmp(child->d_name,"..") == 0) {
            continue;
        }

        // Parse the filename to decide what type of file it is using regex

        // Match files with names starting with UTC string, e.g. 2017-06-14T19:41:09.282Z.pgm
        // These are the raw frames from the sequence
        if(std::regex_search(child->d_name, utcRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path.toStdString() + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the vector
            Image seq;
            std::ifstream input(childPath);
            input >> seq;
            sequence.push_back(seq);
            input.close();
        }

        // Detect the peak hold image
        if(std::regex_search(child->d_name, peakHoldRegex, std::regex_constants::match_continuous)) {
            // Build full path to the item
            std::string childPath = path.toStdString() + "/" + child->d_name;
            // Load the image from file and store a shared pointer to it in the peakHold variable
            std::ifstream input(childPath);
            input >> peakHold;
            input.close();
        }
    }
    closedir (dir);

    // Sort the image sequence into ascending order of capture time
    std::sort(sequence.begin(), sequence.end());


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
