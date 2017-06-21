#include "gui/analysiswidget.h"
#include "gui/glmeteordrawer.h"
#include "infra/analysisinventory.h"

#include <QHBoxLayout>
#include <QPushButton>

AnalysisWidget::AnalysisWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state), inv(0), replay(0) {

    replay = new GLMeteorDrawer(this, this->state, false);

    // TODO: display the usual symbols for each button
    play_button = new QPushButton("Play", this);
    pause_button = new QPushButton("Pause", this);
    stop_button = new QPushButton("Stop", this);
    player = new ReplayVideoThread;

    // A widget to contain the control buttons
    QWidget * controls = new QWidget(this);
    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(play_button);
    controlsLayout->addWidget(pause_button);
    controlsLayout->addWidget(stop_button);
    controls->setLayout(controlsLayout);

    connect(play_button, SIGNAL(pressed()), player, SLOT(play()));
    connect(pause_button, SIGNAL(pressed()), player, SLOT(pause()));
    connect(stop_button, SIGNAL(pressed()), player, SLOT(stop()));

    connect(player, SIGNAL(queueNewFrame(std::shared_ptr<Image>)), replay, SLOT(newFrame(std::shared_ptr<Image>)));

    // Arrange layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(replay);
    mainLayout->addWidget(controls);
    this->setLayout(mainLayout);
}

void AnalysisWidget::loadClip(QString path) {

    // If there's already an AnalysisInventory loaded then delete it
    if(inv) {
        delete inv;
    }

    inv = AnalysisInventory::loadFromDir(path.toStdString());

    if(!inv) {
        // Couldn't load from dir!
        qInfo() << "Couldn't load analysis from " << path;
        return;
    }

    // Pass the clip to the player
    player->loadClip(inv->eventFrames);

    // Initialise it with the peak hold image
    replay->newFrame(inv->peakHold);
}

