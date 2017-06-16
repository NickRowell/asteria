#include "acquisitionwidget.h"
#include "gui/glmeteordrawer.h"

#include <QHBoxLayout>

AcquisitionWidget::AcquisitionWidget(QWidget *parent, AsteriaState * state) : QWidget(parent), state(state) {

    live = new GLMeteorDrawer(this, this->state, true);

    acqThread = new AcquisitionThread(this, this->state);

    // Connect image acquisition signal to image display slot
    connect(acqThread, SIGNAL (acquiredImage(std::shared_ptr<Image>)), live, SLOT (newFrame(std::shared_ptr<Image>)));

    // Forward the acquiredClip signal
    connect(acqThread, SIGNAL (acquiredClip(std::string)), this, SIGNAL (acquiredClip(std::string)));

    // Arrange layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(live);
    this->setLayout(mainLayout);

    acqThread->launch();
}

AcquisitionWidget::~AcquisitionWidget() {
    delete acqThread;
}
