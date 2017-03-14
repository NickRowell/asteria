#include "configcreationwindow.h"
#include "infra/meteorcapturestate.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QTabWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QDebug>

#include "gui/systemtab.h"
#include "gui/stationtab.h"
#include "gui/cameratab.h"

ConfigCreationWindow::ConfigCreationWindow(QWidget *parent, MeteorCaptureState * state) : QDialog(parent), state(state)
{
    tabWidget = new QTabWidget;
    tabWidget->addTab(new SystemTab(state), tr("System parameters"));
    tabWidget->addTab(new StationTab(state), tr("Station parameters"));
    tabWidget->addTab(new CameraTab(state), tr("Camera parameters"));


    QPushButton *load_button = new QPushButton("Load", this);
    QPushButton *save_button = new QPushButton("Save", this);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    buttonBox->addButton(load_button, QDialogButtonBox::ActionRole);
    buttonBox->addButton(save_button, QDialogButtonBox::ActionRole);


    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOkButtonClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotCancelButtonClicked()));
    connect(load_button, SIGNAL(pressed()), this, SLOT(loadConfig()));
    connect(save_button, SIGNAL(pressed()), this, SLOT(saveConfig()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    this->setLayout(mainLayout);
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));
}


void ConfigCreationWindow::loadConfig() {

    QString dir = QFileDialog::getExistingDirectory(this, tr("Select config directory"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    qInfo() << "Got directory " << dir;

    // Load files from config directory. These are likely to be:
    //
    // 1) Main config file, containing key-value pairs
    // 2) Calibration history data, for running calibrations
    // 3) Other data files e.g. Earth geoid data, JPL ephemeris, maps, etc
}

void ConfigCreationWindow::saveConfig() {

}

void ConfigCreationWindow::slotOkButtonClicked() {
    // Verify config and move on to main window
    hide();
    emit ok();
}

void ConfigCreationWindow::slotCancelButtonClicked() {
    // Hide this widget and show camera selection window
    hide();
    emit cancel();
}
