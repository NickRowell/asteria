#ifndef CONFIGCREATIONWINDOW_H
#define CONFIGCREATIONWINDOW_H

#include <QWidget>

class QLineEdit;
class MeteorCaptureState;

class ConfigCreationWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigCreationWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);

private:

    MeteorCaptureState * state;

    // System parameters

    // Camera parameters

    // Station parameters
    QLineEdit * longitudeEntry;
    QLineEdit * latitudeEntry;

signals:

public slots:

    void loadConfig(char * path);
    void saveConfig(char * path);

};

#endif // CONFIGCREATIONWINDOW_H
