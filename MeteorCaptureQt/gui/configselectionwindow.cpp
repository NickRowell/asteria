#include "gui/configselectionwindow.h"
#include "infra/meteorcapturestate.h"

#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QDebug>

ConfigSelectionWindow::ConfigSelectionWindow(QWidget *parent, MeteorCaptureState * state) : QWidget(parent)
{
    this->state = state;

    // Create layout objects
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox1 = new QHBoxLayout;
    QHBoxLayout *hbox2 = new QHBoxLayout;
    QGroupBox *groupBox = new QGroupBox(tr("Load/create configuration"), this);

    // Create content objects
    configLabel = new QLabel(tr("Config:"));
    configLine = new QLineEdit();
    configLine->setReadOnly(true);
    loadButton = new QPushButton("Load config", this);
    createButton = new QPushButton("Create config", this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    // Add content to layouts
    hbox1->addWidget(configLabel);
    hbox1->addWidget(configLine);
    hbox2->addWidget(loadButton);
    hbox2->addWidget(createButton);

    // Arrange layouts
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addStretch(1);
    groupBox->setLayout(vbox);

    mainLayout->addWidget(groupBox);
    mainLayout->addWidget(buttonBox);

    this->setLayout(mainLayout);
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));

    // Receives signal that user clicked the button
    connect(loadButton, SIGNAL (clicked(bool)), this, SLOT (slotLoadButtonClicked()));
    connect(createButton, SIGNAL (clicked(bool)), this, SLOT (slotCreateButtonClicked()));

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOkButtonClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotCancelButtonClicked()));
}

void ConfigSelectionWindow::slotLoadButtonClicked() {

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

void ConfigSelectionWindow::slotCreateButtonClicked() {

    // Present user with ConfigCreationWindow
    // Capture the OK signal from the config creation window

}

void ConfigSelectionWindow::slotOkButtonClicked() {

    // Verify config and move on to main window
    hide();
    emit ok();
}

void ConfigSelectionWindow::slotCancelButtonClicked() {

    // Hide this widtget and show camera selection window
    hide();
    emit cancel();
}
