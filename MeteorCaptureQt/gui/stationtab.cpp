#include "stationtab.h"
#include "infra/meteorcapturestate.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>



StationTab::StationTab(MeteorCaptureState * state, QWidget *parent) : QWidget(parent), state(state)
{
    QLabel *longitudeLabel = new QLabel(tr("Longitude [deg]:"));
    QLineEdit *longitudeEdit = new QLineEdit();
    longitudeEdit->setReadOnly(false);
    //longitudeEdit->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    QLabel *latitudeLabel = new QLabel(tr("Latitude [deg]:"));
    QLineEdit *latitudeEdit = new QLineEdit();
    latitudeEdit->setReadOnly(false);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(longitudeLabel);
    mainLayout->addWidget(longitudeEdit);
    mainLayout->addWidget(latitudeLabel);
    mainLayout->addWidget(latitudeEdit);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}
