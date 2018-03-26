#ifndef CONFIGPARAMETERFAMILYTAB_H
#define CONFIGPARAMETERFAMILYTAB_H

#include "config/configparameterfamily.h"

#include <QWidget>

class ConfigParameterFamilyTab : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigParameterFamilyTab(ConfigParameterFamily *fam, QWidget *parent = 0);

private:
    /**
     * @brief Pointer to the ConfigParameterFamily that this ConfigParameterFamilyTab is presenting.
     */
    ConfigParameterFamily *fam;

signals:

public slots:

    /**
     * @brief Read the parameter values entered in the GUI and check their validity. The parsed values
     * are stored to the internal values of the config parameters.
     * @return
     *  A boolean flag indicating if all the parameters are valid.
     */
    bool readAndValidate();
};

#endif // CONFIGPARAMETERFAMILYTAB_H
