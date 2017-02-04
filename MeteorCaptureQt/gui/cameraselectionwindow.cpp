#include "gui/cameraselectionwindow.h"
#include "infra/meteorcapturestate.h"
#include "util/V4L2Util.h"

#include <QPushButton>
#include <QApplication>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QDebug>
#include <QDesktopWidget>
#include <QButtonGroup>
#include <QString>

CameraSelectionWindow::CameraSelectionWindow(QWidget *parent, MeteorCaptureState * state) : QWidget(parent)
{
    this->state = state;

    // Test

    // Query the available cameras
    cameras = V4L2Util::getCamerasList();

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
    state->cameraPath = new string(camera.first);
    // Open the camera device and store the file descriptor to the state
    state->fd = new int(open(camera.first.c_str(), O_RDWR));

    qInfo() << "Selected camera = " << QString::fromStdString(camera.second);

    qInfo() << "Pixel formats:";

    V4L2Util::printPixelFormats(*(state->fd));



    hide();
    emit cameraSelected(camera.first);
}
