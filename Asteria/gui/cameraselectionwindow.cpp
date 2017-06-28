#include "gui/cameraselectionwindow.h"
#include "infra/asteriastate.h"
#include "util/v4l2util.h"
#include "gui/glmeteordrawer.h"

#include <QPushButton>
#include <QApplication>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QDesktopWidget>
#include <QButtonGroup>
#include <QString>

CameraSelectionWindow::CameraSelectionWindow(QWidget *parent, AsteriaState * state) : QWidget(parent)
{
    this->state = state;

    // Query the supported cameras
    cameras = V4L2Util::getSupportedV4LCameras(AsteriaState::preferredFormats, AsteriaState::preferredFormatsN);

    if(cameras.size() == 0) {
        // Not found any cameras! Can't proceed.
    }

    group = new QButtonGroup(this);

    // Array of radio buttons, one for each camera
    QRadioButton * radios[cameras.size()];

    // Arrange them in a vertical box layout
    QVBoxLayout *vbox = new QVBoxLayout;

    // Initialise the radio buttons used to select the camera
    for(unsigned int p=0; p<cameras.size(); p++) {
        radios[p] = new QRadioButton(QString::fromStdString(cameras[p].second), this);
        vbox->addWidget(radios[p]);
        group->addButton(radios[p], p);
    }
    group->setExclusive(true);
    radios[0]->setChecked(true);

    // Create the camera selection button
    m_button = new QPushButton("Ok", this);
    vbox->addWidget(m_button);

    vbox->addStretch(1);

    QGroupBox *groupBox = new QGroupBox(tr("Select the camera to use:"), this);
    groupBox->setLayout(vbox);
    groupBox->adjustSize();
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));

    // Receives signal that user clicked the button
    connect(m_button, SIGNAL (clicked(bool)), this, SLOT (slotCameraButtonClicked()));
}

/**
 * @brief CameraSelectionWindow::slotButtonClicked
 */
void CameraSelectionWindow::slotCameraButtonClicked()
{
    // Get the ID number of the selected camera in the list
    int p = group->checkedId();

    // Retrieve the info for the selected camera
    pair< string, string > camera = cameras.at(p);

    // Copy path to the camera device
    state->cameraPath = string(camera.first);

    V4L2Util::openCamera(state->cameraPath, state->fd, state->selectedFormat);

    fprintf(stderr, "Selected camera = %s\n", V4L2Util::getCameraName(*(state->fd)).c_str());
    fprintf(stderr, "Selected pixel format = %s\n", V4L2Util::getFourCC(state->selectedFormat).c_str());

    hide();
    emit cameraSelected(camera.first);
}
