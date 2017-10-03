#include "gui/calibrationwidget.h"
#include "infra/calibrationinventory.h"
#include "infra/asteriastate.h"
#include "gui/videodirectorymodel.h"
#include "util/timeutil.h"
#include "gui/videoplayerwidget.h"
#include "gui/glmeteordrawer.h"
#include "gui/referencestarwidget.h"

#ifdef RECALIBRATE
    #include "infra/calibrationworker.h"
#endif

#include <memory>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTreeView>

CalibrationWidget::CalibrationWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state), inv(0), player(0) {

    tree = new QTreeView(this);
    model = new VideoDirectoryModel(state->calibrationDirPath, "Calibrations", tree);
    tree->setModel(model);
    tree->resizeColumnToContents(0);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    player = new VideoPlayerWidget(this, this->state->width, this->state->height, this->state->nominalFramePeriodUs);

    refStarWidget = new ReferenceStarWidget(this, state);

    backgroundImageViewer = new GLMeteorDrawer(this, this->state->width, this->state->height);
    QVBoxLayout *backgroundImageLayout = new QVBoxLayout;
    backgroundImageLayout->addWidget(backgroundImageViewer);
    backgroundImageLayout->addStretch();
    QWidget * backgroundImageWidget = new QWidget(this);
    backgroundImageWidget->setLayout(backgroundImageLayout);

    // Capture right-clicks in the tree view for displaying context menu
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    // Capture double-clicks in the tree view for replaying videos
    connect(tree, SIGNAL (doubleClicked(const QModelIndex)), this, SLOT(loadClip(const QModelIndex)));

    // Use a tabbed widget to display the video footage and calibration images
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->addTab(player, QString("Raw frames"));
    tabWidget->addTab(refStarWidget, QString("Median"));
    tabWidget->addTab(backgroundImageWidget, QString("Background"));

    // Add more tabs for the other calibration
//    tabWidget->addTab(calWidget, QString("Calibration"));

    // Arrange layout

    // Right panel containing the player widget and any additional stuff
    QWidget * rightPanel = new QWidget(this);
    QVBoxLayout *rightPanelLayout = new QVBoxLayout;
    rightPanelLayout->addWidget(tabWidget);
#ifdef RECALIBRATE
    recalibrate_button = new QPushButton("Recalibrate", this);
    connect(recalibrate_button, SIGNAL(pressed()), this, SLOT(recalibrate()));
    rightPanelLayout->addWidget(recalibrate_button);
#endif
    rightPanel->setLayout(rightPanelLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addWidget(rightPanel);

    this->setLayout(mainLayout);
}

void CalibrationWidget::loadClip(const QModelIndex &index) {

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    QString title = item->data(0).toString();
    QString path = item->data(1).toString();

    // Detect if user has double clicked on a node that is not a video clip
    if(!std::regex_match (title.toStdString().c_str(), TimeUtil::timeRegex)) {
        // Not a clip (clips have titles like 01:34:56). Do nothing.
        return;
    }

    // Load the clip for display
    loadClip(path);
}

void CalibrationWidget::loadClip(QString path) {

    // If there's already an AnalysisInventory loaded then delete it
    if(inv) {
        delete inv;
    }

    inv = CalibrationInventory::loadFromDir(path.toStdString());

    if(!inv) {
        // Couldn't load from dir!
        fprintf(stderr, "Couldn't load calibration from %s\n", path.toStdString().c_str());
        return;
    }

    player->loadClip(inv->calibrationFrames, inv->calibrationFrames.front());
    refStarWidget->loadImage(inv->medianImage);
    backgroundImageViewer->newFrame(inv->backgroundImage, false, true, true);
}

#ifdef RECALIBRATE
    void CalibrationWidget::recalibrate() {
        fprintf(stderr, "Recalibrating...\n");
        // If there's no clip loaded, bail
        if(!inv) {
            fprintf(stderr, "No clip to analyse!\n");
            return;
        }
        QThread* thread = new QThread;
        CalibrationWorker* worker = new CalibrationWorker(NULL, this->state, inv->calibrationFrames);
        worker->moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished(std::string)), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished(std::string)), worker, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished(std::string)), this, SLOT(recalibrationComplete(std::string)));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }

    void CalibrationWidget::recalibrationComplete(std::string utc) {
        fprintf(stderr, "Finished recalibrating %s\n", utc.c_str());
    }
#endif


void CalibrationWidget::onCustomContextMenu(const QPoint &point) {
    QModelIndex index = tree->indexAt(point);
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        item->getContextMenu()->exec(tree->viewport()->mapToGlobal(point));
    }
}
