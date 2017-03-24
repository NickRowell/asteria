#include "configparameterfamilytab.h"

#include "config/configparameter.h"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QDebug>
#include <QToolTip>

ConfigParameterFamilyTab::ConfigParameterFamilyTab(ConfigParameterFamily *fam, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Create fields for each parameter
    for(unsigned int parOff = 0; parOff < fam->numPar; parOff++) {
        ConfigParameter * par = fam->parameters[parOff];

        // Create label and line edit for the parameter
        QLabel *label = new QLabel(tr(par->key.c_str()));
        QLineEdit *edit = new QLineEdit();
        edit->setReadOnly(false);

        std::pair< QLineEdit *, ConfigParameter * > c;
        c.first = edit;
        c.second = par;
        links.push_back(c);

        mainLayout->addWidget(label);
        mainLayout->addWidget(edit);
    }

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

bool ConfigParameterFamilyTab::readAndValidate() {

    bool isValid = true;

    // Iterate over the line edit / parameter links
    for(unsigned int p=0; p<links.size(); p++) {

        QLineEdit * edit = links[p].first;
        ConfigParameter * par = links[p].second;

        // Read the string entered into the line edit and convert to const char *
        QString entry = links[p].first->text();
        QByteArray ba = entry.toLatin1();
        const char *c_entry = ba.data();

        // Pass it to the parameter for validation and assignment
        par->validate(c_entry);

        // Check if it passed and incidate in the form if not
        switch(par->isValid) {
        case VALID: {
            // Clear any red borders for previously invalid parameters
            edit->setStyleSheet("");
            edit->setToolTip(tr(""));
            break;
        }
        case WARNING: {
            edit->setStyleSheet("QLineEdit{border: 2px solid yellow}");
            edit->setToolTip(tr(par->message.c_str()));
            break;
        }
        case INVALID: {
            // Indicate if an parameter failed validation
            isValid = false;
            // Indicate on the form by turning borders red
            edit->setStyleSheet("QLineEdit{border: 2px solid red}");
            edit->setToolTip(tr(par->message.c_str()));
            break;
        }
        }

    }

    return isValid;
}
