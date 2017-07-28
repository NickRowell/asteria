#include "gui/analysiswidget.h"
#include "gui/glmeteordrawer.h"
#include "infra/analysisinventory.h"
#include "infra/asteriastate.h"

#include <memory>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>

AnalysisWidget::AnalysisWidget(QWidget *parent, AsteriaState *state) : QWidget(parent), state(state), inv(0), display(0) {

    display = new GLMeteorDrawer(this, this->state);

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
    slider = new QSlider(Qt::Horizontal, this);
    dicheckbox = new QCheckBox("&De-interlaced stepping", this);

    replayThread = new ReplayVideoThread(this->state->nominalFramePeriodUs);

    // A widget to contain the control buttons
    QWidget * controls = new QWidget(this);
    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(play_button);
    controlsLayout->addWidget(pause_button);
    controlsLayout->addWidget(stop_button);
    controlsLayout->addWidget(stepb_button);
    controlsLayout->addWidget(stepf_button);
    controlsLayout->addWidget(dicheckbox);
    controls->setLayout(controlsLayout);

    // A widget to present the stats of the video clip
    clipLengthSecsField = new QLabel("");
    clipLengthFramesField = new QLabel("");
    utcField = new QLabel("");

    QWidget * statsDisplay = new QWidget(this);
    QHBoxLayout *statsLayout = new QHBoxLayout;
    statsLayout->addWidget(clipLengthSecsField);
    statsLayout->addWidget(clipLengthFramesField);
    statsDisplay->setLayout(statsLayout);

    QWidget * utcDisplay = new QWidget(this);
    QHBoxLayout *utcLayout = new QHBoxLayout;
    utcLayout->addWidget(utcField);
    utcDisplay->setLayout(utcLayout);

    connect(play_button, SIGNAL(pressed()), replayThread, SLOT(play()));
    connect(pause_button, SIGNAL(pressed()), replayThread, SLOT(pause()));
    connect(stop_button, SIGNAL(pressed()), replayThread, SLOT(stop()));
    connect(stepb_button, SIGNAL(pressed()), replayThread, SLOT(stepb()));
    connect(stepf_button, SIGNAL(pressed()), replayThread, SLOT(stepf()));
    connect(dicheckbox, SIGNAL(stateChanged(int)), replayThread, SLOT(toggleDiStepping(int)));

    // Slider response to user actions in the player
    connect(replayThread, SIGNAL(queuedFrameIndex(int)), slider, SLOT(setValue(int)));

    // Player response to user moving the slider
    connect(slider, SIGNAL(sliderMoved(int)), replayThread, SLOT(queueFrameIndex(int)));

    // Display image when one is queued
    connect(replayThread, SIGNAL(queueNewFrame(std::shared_ptr<Image>, bool, bool)), display, SLOT(newFrame(std::shared_ptr<Image>, bool, bool)));
    connect(replayThread, SIGNAL (videoStats(const AnalysisVideoStats &)), this, SLOT (updateVideoStats(const AnalysisVideoStats &)));

    // Arrange layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(display);
    mainLayout->addWidget(statsDisplay);
    mainLayout->addWidget(utcDisplay);
    mainLayout->addWidget(slider);
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
        fprintf(stderr, "Couldn't load analysis from %s\n", path.toStdString().c_str());
        return;
    }

    // Pass the clip to the player
    replayThread->loadClip(inv->eventFrames, inv->peakHold);

    // Set the range of the slider according to how many frames we have
    slider->setRange(0, inv->eventFrames.size()-1);
    slider->setValue(0);

    // Enable/disable the de-interlaced stepping checkbox depending on whether the clip consists of
    // interlaced-scan type images
    switch(inv->eventFrames[0]->field) {
    case V4L2_FIELD_NONE:
        // progressive format; not interlaced
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(false);
        break;
    case V4L2_FIELD_INTERLACED:
        // interleaved/interlaced format
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(true);
        break;
    case V4L2_FIELD_INTERLACED_TB:
        // interleaved/interlaced format; top field is transmitted first
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(true);
        break;
    case V4L2_FIELD_INTERLACED_BT:
        // interleaved/interlaced format; bottom field is transmitted first
        dicheckbox->setChecked(false);
        dicheckbox->setEnabled(true);
        break;
    }

    // Initialise it with the peak hold image
    display->newFrame(inv->peakHold, true, true);
}

void AnalysisWidget::updateVideoStats(const AnalysisVideoStats &stats) {
    utcField->setText(QString::fromStdString(stats.utc));
    QString clipLengthSecsStr;
    clipLengthSecsStr.sprintf("%05.2f / %05.2f", stats.framePositionSecs, stats.clipLengthSecs);
    clipLengthSecsField->setText(clipLengthSecsStr);

    // Unicode symbols to use when displaying full frame, top field & bottom field
    QString both = QString::fromUtf8("\u25CF");
    QString top = QString::fromUtf8("\u25D3");
    QString bottom = QString::fromUtf8("\u25D2");

    QString symbol = (stats.isTopField && stats.isBottomField) ? both : (stats.isTopField ? top : bottom);
    clipLengthFramesField->setText(QString("%1 %2 / %3").arg(QString::number(stats.framePositionFrames), symbol, QString::number(stats.clipLengthFrames)));
}
