#include "acquisitionwidget.h"
#include "gui/glmeteordrawer.h"

#include <sstream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

AcquisitionWidget::AcquisitionWidget(QWidget *parent, AsteriaState * state) : QWidget(parent), state(state) {

    live = new GLMeteorDrawer(this, this->state, true);

    acqThread = new AcquisitionThread(this, this->state);

    // Display the usual symbols for each button
    QIcon playIcon(":/images/play.png");
    QIcon pauseIcon(":/images/stop.png");
    QIcon detectIcon(":/images/detect.png");

    play_button = new QPushButton(this);
    play_button->setIcon(playIcon);
    pause_button = new QPushButton(this);
    pause_button->setIcon(pauseIcon);
    detect_button = new QPushButton(this);
    detect_button->setIcon(detectIcon);

    // A widget to contain the control buttons
//    QWidget * controls = new QWidget(this);
//    QHBoxLayout * controlsLayout = new QHBoxLayout;
//    controlsLayout->addWidget(play_button);
//    controlsLayout->addWidget(pause_button);
//    controlsLayout->addWidget(detect_button);
//    controls->setLayout(controlsLayout);

    QLabel * acqStateLabel = new QLabel("Acquisition state: ");
    acqStateField = new QLabel("");
    QLabel * fpsLabel = new QLabel("Current FPS: ");
    fpsField = new QLabel("");
    QLabel * totalFramesLabel = new QLabel("Total frames: ");
    totalFramesField = new QLabel("");
    QLabel * droppedFramesLabel = new QLabel("Dropped frames: ");
    droppedFramesField = new QLabel("");

    QWidget * acqStateDisplay = new QWidget(this);

//    QHBoxLayout * acqStateDisplayLayout = new QHBoxLayout;
//    acqStateDisplayLayout->addWidget(acqStateLabel);
//    acqStateDisplayLayout->addWidget(acqStateField);
//    acqStateDisplayLayout->addWidget(fpsLabel);
//    acqStateDisplayLayout->addWidget(fpsField);
//    acqStateDisplayLayout->addWidget(totalFramesLabel);
//    acqStateDisplayLayout->addWidget(totalFramesField);
//    acqStateDisplayLayout->addWidget(droppedFramesLabel);
//    acqStateDisplayLayout->addWidget(droppedFramesField);

//    acqStateDisplay->setLayout(acqStateDisplayLayout);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(acqStateLabel, 0, 0);
    layout->addWidget(acqStateField, 0, 1);
    layout->addWidget(new QLabel(""), 0, 2);
    layout->addWidget(fpsLabel, 1, 0);
    layout->addWidget(fpsField, 1, 1);
    layout->addWidget(play_button, 1, 2);
    layout->addWidget(totalFramesLabel, 2, 0);
    layout->addWidget(totalFramesField, 2, 1);
    layout->addWidget(pause_button, 2, 2);
    layout->addWidget(droppedFramesLabel, 3, 0);
    layout->addWidget(droppedFramesField, 3, 1);
    layout->addWidget(detect_button, 3, 2);

    acqStateDisplay->setLayout(layout);

    connect(play_button, SIGNAL(pressed()), acqThread, SLOT(preview()));
    connect(pause_button, SIGNAL(pressed()), acqThread, SLOT(pause()));
    connect(detect_button, SIGNAL(pressed()), acqThread, SLOT(detect()));

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>)), live, SLOT (newFrame(std::shared_ptr<Image>)));

    // Forward the acquiredClip signal
    connect(acqThread, SIGNAL (acquiredClip(std::string)), this, SIGNAL (acquiredClip(std::string)));

    connect(acqThread, SIGNAL (transitionedToState(AcquisitionThread::AcquisitionState)), this, SLOT (updateAcquisitionState(AcquisitionThread::AcquisitionState)));
    connect(acqThread, SIGNAL (videoStats(const VideoStats &)), this, SLOT (updateVideoStats(const VideoStats &)));

    // Arrange layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(live);
    mainLayout->addWidget(acqStateDisplay);
//    mainLayout->addWidget(controls);
    this->setLayout(mainLayout);

    acqThread->launch();
}

AcquisitionWidget::~AcquisitionWidget() {
    delete acqThread;
}

void AcquisitionWidget::updateAcquisitionState(AcquisitionThread::AcquisitionState acqState) {

    std::ostringstream oss;
    oss << "<b>" << AcquisitionThread::acquisitionStateNames[acqState] << "</b>";

    acqStateField->setText(QString::fromStdString(oss.str()));
}

void AcquisitionWidget::updateVideoStats(const VideoStats &stats) {
    fpsField->setText(QString::asprintf("%5.3f", stats.fps));
    totalFramesField->setText(QString::asprintf("%5d", stats.totalFrames));
    droppedFramesField->setText(QString::asprintf("%5d", stats.droppedFrames));
}
