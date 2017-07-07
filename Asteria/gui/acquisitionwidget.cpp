#include "acquisitionwidget.h"
#include "gui/glmeteordrawer.h"

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
    QWidget * controls = new QWidget(this);
    QHBoxLayout * controlsLayout = new QHBoxLayout;
    controlsLayout->addWidget(play_button);
    controlsLayout->addWidget(pause_button);
    controlsLayout->addWidget(detect_button);
    controls->setLayout(controlsLayout);

    QLabel * acqStateLabel = new QLabel("Acquisition state: ");
    acqStateField = new QLabel("");
    QWidget * acqStateDisplay = new QWidget(this);
    QHBoxLayout * acqStateDisplayLayout = new QHBoxLayout;
    acqStateDisplayLayout->addWidget(acqStateLabel);
    acqStateDisplayLayout->addWidget(acqStateField);
    acqStateDisplay->setLayout(acqStateDisplayLayout);

    connect(play_button, SIGNAL(pressed()), acqThread, SLOT(preview()));
    connect(pause_button, SIGNAL(pressed()), acqThread, SLOT(pause()));
    connect(detect_button, SIGNAL(pressed()), acqThread, SLOT(detect()));

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>)), live, SLOT (newFrame(std::shared_ptr<Image>)));

    // Forward the acquiredClip signal
    connect(acqThread, SIGNAL (acquiredClip(std::string)), this, SIGNAL (acquiredClip(std::string)));

    connect(acqThread, SIGNAL (transitionedToState(AcquisitionThread::AcquisitionState)), this, SLOT (updateAcquisitionState(AcquisitionThread::AcquisitionState)));

    // Arrange layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(live);
    mainLayout->addWidget(acqStateDisplay);
    mainLayout->addWidget(controls);
    this->setLayout(mainLayout);

    acqThread->launch();
}

AcquisitionWidget::~AcquisitionWidget() {
    delete acqThread;
}

void AcquisitionWidget::updateAcquisitionState(AcquisitionThread::AcquisitionState acqState) {
    acqStateField->setText(QString::fromStdString(AcquisitionThread::acquisitionStateNames[acqState]));
}
