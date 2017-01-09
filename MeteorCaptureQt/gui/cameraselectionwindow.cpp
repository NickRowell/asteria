#include "gui/cameraselectionwindow.h"
#include "infra/meteorcapturestate.h"

#include <QPushButton>
#include <QApplication>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QDesktopWidget>
#include <QButtonGroup>

CameraSelectionWindow::CameraSelectionWindow(QWidget *parent, MeteorCaptureState * state) : QWidget(parent)
{
    this->state = state;

    // Query the available cameras
    cameras = QCameraInfo::availableCameras();

    if(cameras.size() == 0) {
        // Not found any cameras! Can't proceed.
    }

    group = new QButtonGroup(this);

    // Array of radio buttons, one for each camera
    QRadioButton * radios[cameras.size()];

    // Arrange them in a vertical box layout
    QVBoxLayout *vbox = new QVBoxLayout;

    // Initialise the radio buttons used to select the camera
    int p=0;
    foreach (const QCameraInfo &cameraInfo, cameras) {
        radios[p] = new QRadioButton(cameraInfo.description(), this);
        vbox->addWidget(radios[p]);
        group->addButton(radios[p], p);
        p++;
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
    QCameraInfo camInfo = cameras.at(p);

    // Copy QCameraInfo to dynamic memory and store pointer in the state object
    state->qCameraInfo = new QCameraInfo(camInfo);

    qInfo() << "Selected camera = " << camInfo.description();

    hide();
    emit cameraSelected(camInfo);
}
