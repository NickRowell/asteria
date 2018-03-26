#include "configcreationwindow.h"
#include "infra/asteriastate.h"

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

#include "config/configstore.h"
#include "gui/configparameterfamilytab.h"

ConfigCreationWindow::ConfigCreationWindow(QWidget *parent, AsteriaState * state) : QDialog(parent), state(state)
{
    tabWidget = new QTabWidget;

    store = new ConfigStore(state);
    tabs = new ConfigParameterFamilyTab *[store->numFamilies];

    // Add one tabbed widget per parameter family present in the config
    for(unsigned int famOff = 0; famOff < store->numFamilies; famOff++) {
        // Build the GUI
        ConfigParameterFamily * fam = store->families[famOff];
        ConfigParameterFamilyTab * tab = new ConfigParameterFamilyTab(fam, tabWidget);
        tabs[famOff] = tab;
        tabWidget->addTab(tab, QString("%1 Parameters:").arg(fam->title.c_str()));
    }

    QPushButton *load_button = new QPushButton("Load", this);
    QPushButton *save_button = new QPushButton("Save", this);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    buttonBox->addButton(load_button, QDialogButtonBox::ActionRole);
    buttonBox->addButton(save_button, QDialogButtonBox::ActionRole);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(okClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelClicked()));
    connect(load_button, SIGNAL(pressed()), this, SLOT(loadClicked()));
    connect(save_button, SIGNAL(pressed()), this, SLOT(saveClicked()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    this->setLayout(mainLayout);
    this->adjustSize();

    // Rectangle aligned with the screen, for use in centering the widget
    const QRect rect = qApp->desktop()->availableGeometry();
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), rect));
}

ConfigCreationWindow::~ConfigCreationWindow() {

    // Delete the ConfigParameterFamilyTabs
    for(unsigned int famOff = 0; famOff < store->numFamilies; famOff++) {
        delete tabs[famOff];
    }

    // Delete the ConfigStore
    delete store;
}

// Read files in the config directory, pick out the parameters and load them into the GUI form
void ConfigCreationWindow::loadClicked() {
    QString path = QFileDialog::getOpenFileName(this, tr("Select asteria.config file"), "", tr("Config Files (*.config)"));
    string pathStr = path.toUtf8().constData();
    store->loadFromFile(pathStr);
}

// Write parameters from the GUI form to disk
void ConfigCreationWindow::saveClicked() {

    // Check if ALL the parameter families are valid
    bool allValid = true;

    // Verify each configparameterfamilytab in turn
    for(unsigned int famOff = 0; famOff < store->numFamilies; famOff++) {
        bool valid = tabs[famOff]->readAndValidate();
        if(!valid) {
            allValid = false;
        }
    }

    if(allValid) {
        // Write the parameters to a file in the configuration directory
        // TODO: detect if a trailing slash is needed on the configDirPath
        string configFilePath = state->configDirPath + "/asteria.config";
        store->saveToFile(configFilePath);
    }
}

// Read parameters from the GUI, verify them and store them in the state
void ConfigCreationWindow::okClicked() {

    // Check if ALL the parameter families are valid
    bool allValid = true;

    // Verify each configparameterfamilytab in turn
    for(unsigned int famOff = 0; famOff < store->numFamilies; famOff++) {
        bool valid = tabs[famOff]->readAndValidate();
        if(!valid) {
            allValid = false;
        }
    }

    if(allValid) {
        // Verify config and move on to main window
        hide();
        emit ok();
    }
}

void ConfigCreationWindow::cancelClicked() {
    // Hide this widget and show camera selection window
    hide();
    emit cancel();
}
