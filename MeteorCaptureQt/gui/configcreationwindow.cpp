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

//#include <QPushButton>
//#include <QDialogButtonBox>
//#include <QDebug>




ConfigCreationWindow::ConfigCreationWindow(QWidget *parent, MeteorCaptureState * state) : QWidget(parent)
{
    this->state = state;

    // Create layout objects
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox1 = new QHBoxLayout;
    QHBoxLayout *hbox2 = new QHBoxLayout;
    QGroupBox *groupBoxStat = new QGroupBox(tr("Station parameters"), this);
    QGroupBox *groupBoxSys = new QGroupBox(tr("System parameters"), this);
    QGroupBox *groupBoxCam = new QGroupBox(tr("Camera parameters"), this);


    // Create content objects
    longitudeEntry = new QLineEdit();
    longitudeEntry->setReadOnly(false);

    latitudeEntry = new QLineEdit();
    latitudeEntry->setReadOnly(false);



//    loadButton = new QPushButton("Load config", this);
//    createButton = new QPushButton("Create config", this);
//    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
//                                         | QDialogButtonBox::Cancel);

    // Add content to layouts
    QLabel * longLabel = new QLabel(tr("Longitude [deg]"));
    QLabel * latLabel = new QLabel(tr("Latitude [deg]"));

    hbox1->addWidget(longLabel);
    hbox1->addWidget(longitudeEntry);
    hbox2->addWidget(latLabel);
    hbox2->addWidget(latitudeEntry);

    // Arrange layouts
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addStretch(1);
    groupBoxStat->setLayout(vbox);

    mainLayout->addWidget(groupBoxSys);
    mainLayout->addWidget(groupBoxStat);
    mainLayout->addWidget(groupBoxCam);
//    mainLayout->addWidget(buttonBox);

    this->setLayout(mainLayout);
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));







}


void ConfigCreationWindow::loadConfig(char * path) {

}

void ConfigCreationWindow::saveConfig(char * path) {

}
