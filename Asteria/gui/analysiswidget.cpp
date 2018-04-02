#include "gui/analysiswidget.h"
#include "infra/analysisinventory.h"
#include "infra/asteriastate.h"
#include "gui/videodirectorymodel.h"
#include "util/timeutil.h"
#include "gui/videoplayerwidget.h"

#ifdef REANALYSE
    #include "infra/analysisworker.h"
#endif

#include <memory>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTreeView>

AnalysisWidget::AnalysisWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state), inv(0), player(0) {

    tree = new QTreeView(this);
    model = new VideoDirectoryModel(state->videoDirPath, "Video clips", tree);
    tree->setModel(model);
    tree->resizeColumnToContents(0);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    player = new VideoPlayerWidget(this, this->state->width, this->state->height, this->state->nominalFramePeriodUs);

    // Capture right-clicks in the tree view for displaying context menu
    connect(tree, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));

    // Capture double-clicks in the tree view for replaying videos
    connect(tree, SIGNAL (doubleClicked(const QModelIndex)), this, SLOT(loadClip(const QModelIndex)));

    // Arrange layout

    // Right panel containing the player widget and any additional stuff
    QWidget * rightPanel = new QWidget(this);
    QVBoxLayout *rightPanelLayout = new QVBoxLayout;
    rightPanelLayout->addWidget(player);
#ifdef REANALYSE
    reanalyse_button = new QPushButton("Reanalyse", this);
    connect(reanalyse_button, SIGNAL(pressed()), this, SLOT(reanalyse()));
    rightPanelLayout->addWidget(reanalyse_button);
#endif
    rightPanel->setLayout(rightPanelLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tree);
    mainLayout->addWidget(rightPanel);

    this->setLayout(mainLayout);
}

void AnalysisWidget::loadClip(const QModelIndex &index) {

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

void AnalysisWidget::loadClip(QString path) {

    // If there's already an AnalysisInventory loaded then delete it
    if(inv) {
        delete inv;
    }

    inv = AnalysisInventory::loadFromDir(path.toStdString());

    if(!inv) {
        // Couldn't load from dir!
        fprintf(stderr, "Couldn't load analysis from %s\n", path.toStdString().c_str());
        return;
    }

    player->loadClip(inv->eventFrames, inv->peakHold);
}

#ifdef REANALYSE
    void AnalysisWidget::reanalyse() {
        fprintf(stderr, "Reanalysing...\n");
        // If there's no clip loaded, bail
        if(!inv) {
            fprintf(stderr, "No clip to analyse!\n");
            return;
        }
        QThread* thread = new QThread;
        // TODO: reanalyse using specific calibration and not the one currently loaded in the state object, which may be inappropriate
        AnalysisWorker* worker = new AnalysisWorker(NULL, this->state, this->state->cal, inv->eventFrames);
        worker->moveToThread(thread);
        connect(thread, SIGNAL(started()), worker, SLOT(process()));
        connect(worker, SIGNAL(finished(std::string)), thread, SLOT(quit()));
        connect(worker, SIGNAL(finished(std::string)), worker, SLOT(deleteLater()));
        connect(worker, SIGNAL(finished(std::string)), this, SLOT(reanalysisComplete(std::string)));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }

    void AnalysisWidget::reanalysisComplete(std::string utc) {
        fprintf(stderr, "Finished reanalysing %s\n", utc.c_str());
    }
#endif


void AnalysisWidget::onCustomContextMenu(const QPoint &point) {
    QModelIndex index = tree->indexAt(point);
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        item->getContextMenu()->exec(tree->viewport()->mapToGlobal(point));
    }
}
