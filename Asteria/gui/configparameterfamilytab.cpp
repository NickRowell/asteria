#include "gui/configparameterfamilytab.h"
#include "config/configparameterbase.h"
#include "config/parametermultiplechoice.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>

ConfigParameterFamilyTab::ConfigParameterFamilyTab(ConfigParameterFamily *fam, QWidget *parent) : QWidget(parent), fam(fam)
{

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Insert fields of each parameter
    for(unsigned int parOff = 0; parOff < fam->numPar; parOff++) {
        ConfigParameterBase * par = fam->parameters[parOff];
        QWidget * widget = par->entryField;

        // Create label for the parameter
        string parLabel = par->title;
        if(!par->units.empty()) {
            parLabel += " [" + par->units + "]";
        }
        QLabel *label = new QLabel(tr(parLabel.c_str()));

        mainLayout->addWidget(label);
        mainLayout->addWidget(widget);
    }

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

bool ConfigParameterFamilyTab::readAndValidate() {

    bool isValid = true;

    for(unsigned int parOff = 0; parOff < fam->numPar; parOff++) {
        ConfigParameterBase * par = fam->parameters[parOff];
        par->parseAndValidate();
        // Check if it passed and incidate in the form if not
        if(par->isValid == INVALID) {
            isValid = false;
        }
    }
    return isValid;
}
