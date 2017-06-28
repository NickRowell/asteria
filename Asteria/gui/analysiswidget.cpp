#include "gui/analysiswidget.h"
#include "gui/glmeteordrawer.h"
#include "infra/analysisinventory.h"

#include <QHBoxLayout>
#include <QPushButton>

AnalysisWidget::AnalysisWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state), inv(0), replay(0) {

    replay = new GLMeteorDrawer(this, this->state, false);

    // Display the usual symbols for each button
    QIcon playIcon(":/images/play.png");
    QIcon pauseIcon(":/images/pause.png");
    QIcon stopIcon(":/images/stop.png");
    QIcon stepfIcon(":/images/stepf.png");
    QIcon stepbIcon(":/images/stepb.png");

    play_button = new QPushButton(this);
    play_button->setIcon(playIcon);
    pause_button = new QPushButton(this);
    pause_button->setIcon(pauseIcon);
    stop_button = new QPushButton(this);
    stop_button->setIcon(stopIcon);
    stepb_button = new QPushButton(this);
    stepb_button->setIcon(stepbIcon);
    stepf_button = new QPushButton(this);
    stepf_button->setIcon(stepfIcon);

    player = new ReplayVideoThread;

    // A widget to contain the control buttons
    QWidget * controls = new QWidget(this);
    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(play_button);
    controlsLayout->addWidget(pause_button);
    controlsLayout->addWidget(stop_button);
    controlsLayout->addWidget(stepb_button);
    controlsLayout->addWidget(stepf_button);
    controls->setLayout(controlsLayout);

    connect(play_button, SIGNAL(pressed()), player, SLOT(play()));
    connect(pause_button, SIGNAL(pressed()), player, SLOT(pause()));
    connect(stop_button, SIGNAL(pressed()), player, SLOT(stop()));
    connect(stepb_button, SIGNAL(pressed()), player, SLOT(stepb()));
    connect(stepf_button, SIGNAL(pressed()), player, SLOT(stepf()));

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

