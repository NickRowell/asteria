#include "gui/mainwindow.h"
#include "infra/asteriastate.h"
#include "gui/acquisitionwidget.h"
#include "gui/analysiswidget.h"
#include "gui/calibrationwidget.h"
#include "gui/videodirectorymodel.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QCloseEvent>
#include <QGridLayout>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent, AsteriaState * state) : QMainWindow(parent), state(state) {
    // Nothing to do; intialistion is performed later once the camera connection is opened and the
    // configuration parameters are loaded.
}

void MainWindow::initAndShowGui() {

    // Initialisation to perform:
    // 1) Create the main GUI components: acquisition, analysis and calibration tabs
    // 2) Load the video directory contents into the viewer
    // 3) Connect all signals/slots

    acqWidget = new AcquisitionWidget(this, this->state);
    analWidget = new AnalysisWidget(this, this->state);
    calWidget = new CalibrationWidget(this, this->state);

    tabWidget = new QTabWidget;
    tabWidget->addTab(acqWidget, QString("Acquisition"));
    tabWidget->addTab(analWidget, QString("Analysis"));
    tabWidget->addTab(calWidget, QString("Calibration"));

    // Arrange layout
    central = new QWidget(this);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tabWidget);
    central->setLayout(mainLayout);

    this->setCentralWidget(central);

    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));

    // Connect new clip signal to tree viewer slot, so that new clips get added to the viewer
    connect(acqWidget, SIGNAL (acquiredClip(std::string)), analWidget->model, SLOT (addNewClipByUtc(std::string)));
    connect(acqWidget, SIGNAL (acquiredCalibration(std::string)), calWidget->model, SLOT (addNewClipByUtc(std::string)));

    show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Any cleaning up to do?
}
