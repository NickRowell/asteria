#include "gui/mainwindow.h"
#include "infra/asteriastate.h"
#include "gui/videodirectorymodel.h"
#include "gui/acquisitionwidget.h"
#include "gui/analysiswidget.h"
#include "util/timeutil.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QCloseEvent>
#include <QGridLayout>
#include <QTreeView>


MainWindow::MainWindow(QWidget *parent, AsteriaState * state) : QMainWindow(parent), state(state) {
    // Nothing to do; intialistion is performed later once the camera connection is opened and the
    // configuration parameters are loaded.
}

void MainWindow::initAndShowGui() {

    // Initialisation to perform:
    // 1) Create the main GUI components: acquisition, analysis and calibration tabs
    // 2) Load the video directory contents into the viewer
    // 3) Connect all signals/slots

    acqWidget = new AcquisitionWidget(this, this->state);
    analWidget = new AnalysisWidget(this, this->state);

    tabWidget = new QTabWidget;
    tabWidget->addTab(acqWidget, QString("Live"));
    tabWidget->addTab(analWidget, QString("Replay"));

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

    // Connect new clip signal to tree viewer slot, so that new clips get added to the viewer
    connect(acqWidget, SIGNAL (acquiredClip(std::string)), model, SLOT (addNewClipByUtc(std::string)));

    show();
}


void MainWindow::replayVideo(const QModelIndex &index) {

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    QString title = item->data(0).toString();
    QString path = item->data(1).toString();

    // Detect if user has double clicked on a node that is not a video clip
    if(!std::regex_match (title.toStdString().c_str(), TimeUtil::getTimeRegex())) {
        // Not a clip (clips have titles like 01:34:56). Do nothing.
        return;
    }

    // Show the replay tab
    tabWidget->setCurrentIndex(1);

    // Load the clip for display
    analWidget->loadClip(path);
}

void MainWindow::onCustomContextMenu(const QPoint &point) {
    QModelIndex index = tree->indexAt(point);
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        item->getContextMenu()->exec(tree->viewport()->mapToGlobal(point));
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Any cleaning up to do?
}
