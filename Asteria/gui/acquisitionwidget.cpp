#include "acquisitionwidget.h"
#include "gui/glmeteordrawer.h"

#include <sstream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

AcquisitionWidget::AcquisitionWidget(QWidget *parent, AsteriaState * state) : QWidget(parent), state(state) {

    display = new GLMeteorDrawer(this, this->state);

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

    QLabel * acqStateLabel = new QLabel("Acquisition state: ");
    acqStateField = new QLabel("");
    QLabel * utcLabel = new QLabel("UTC: ");
    utcField = new QLabel("");
    QLabel * fpsLabel = new QLabel("Current FPS: ");
    fpsField = new QLabel("");
    QLabel * totalFramesLabel = new QLabel("Total frames: ");
    totalFramesField = new QLabel("");
    QLabel * droppedFramesLabel = new QLabel("Dropped frames: ");
    droppedFramesField = new QLabel("");

    QWidget * acqStateDisplay = new QWidget(this);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(acqStateLabel, 0, 0);
    layout->addWidget(acqStateField, 0, 1);
    layout->addWidget(new QLabel(""), 0, 2);
    layout->addWidget(utcLabel, 1, 0);
    layout->addWidget(utcField, 1, 1);
    layout->addWidget(new QLabel(""), 1, 2);
    layout->addWidget(fpsLabel, 2, 0);
    layout->addWidget(fpsField, 2, 1);
    layout->addWidget(play_button, 2, 2);
    layout->addWidget(totalFramesLabel, 3, 0);
    layout->addWidget(totalFramesField, 3, 1);
    layout->addWidget(pause_button, 3, 2);
    layout->addWidget(droppedFramesLabel, 4, 0);
    layout->addWidget(droppedFramesField, 4, 1);
    layout->addWidget(detect_button, 4, 2);

    acqStateDisplay->setLayout(layout);

    connect(play_button, SIGNAL(pressed()), acqThread, SLOT(preview()));
    connect(pause_button, SIGNAL(pressed()), acqThread, SLOT(pause()));
    connect(detect_button, SIGNAL(pressed()), acqThread, SLOT(detect()));

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>, bool, bool, bool)), display, SLOT (newFrame(std::shared_ptr<Image>, bool, bool, bool)));

    // Forward the acquiredClip signal
    connect(acqThread, SIGNAL (acquiredClip(std::string)), this, SIGNAL (acquiredClip(std::string)));

    connect(acqThread, SIGNAL (transitionedToState(AcquisitionThread::AcquisitionState)), this, SLOT (updateAcquisitionState(AcquisitionThread::AcquisitionState)));
    connect(acqThread, SIGNAL (videoStats(const AcquisitionVideoStats &)), this, SLOT (updateVideoStats(const AcquisitionVideoStats &)));

    // Arrange layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(display);
    mainLayout->addWidget(acqStateDisplay);
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

void AcquisitionWidget::updateVideoStats(const AcquisitionVideoStats &stats) {
    utcField->setText(QString::fromStdString(stats.utc));
    fpsField->setText(QString::asprintf("%5.3f", stats.fps));
    totalFramesField->setText(QString::asprintf("%5d", stats.totalFrames));
    droppedFramesField->setText(QString::asprintf("%5d", stats.droppedFrames));
}
