#ifndef STATIONTAB_H
#define STATIONTAB_H

#include <QWidget>

class MeteorCaptureState;

class StationTab : public QWidget
{
    Q_OBJECT
public:
    explicit StationTab(MeteorCaptureState * state = 0, QWidget *parent = 0);

    // Read parameters entered in the form
    void readParameters();

    // Write parameters to the form
    void writeParameters();

    // Load parameters from configuration file
    void loadParameters();

    // Write parameters for configuration file
    void saveParameters(const char * file);

    // Validate parameters entered in the form
    void validateParameters();

signals:

public slots:

private:
    MeteorCaptureState *state;
};

#endif // STATIONTAB_H
