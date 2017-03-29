#ifndef CONFIGPARAMETERFAMILYTAB_H
#define CONFIGPARAMETERFAMILYTAB_H

#include <QWidget>
#include <QLineEdit>

#include <vector>
#include <utility>    // provides pair

#include "config/configparameterfamily.h"

class ConfigParameterFamilyTab : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigParameterFamilyTab(ConfigParameterFamily *fam, QWidget *parent = 0);

private:
    /**
     * @brief Used to maintain link between the line entry fields (that we use to read/write parameters) and the
     * parameters in memory.
     */
    std::vector< std::pair< QLineEdit *, ConfigParameter * > > links;

signals:


public slots:

    void updateForm();

    bool readAndValidate();

};

#endif // CONFIGPARAMETERFAMILYTAB_H
