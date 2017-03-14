#ifndef CONFIGCREATIONWINDOW_H
#define CONFIGCREATIONWINDOW_H

#include <QDialog>

class QDialogButtonBox;
class QTabWidget;
class QLineEdit;
class MeteorCaptureState;

class ConfigCreationWindow : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigCreationWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);

private:

    MeteorCaptureState *state;

    QTabWidget *tabWidget;
    QDialogButtonBox *buttonBox;

signals:
    void ok();
    void cancel();

public slots:

    void loadConfig();
    void saveConfig();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();

};

#endif // CONFIGCREATIONWINDOW_H
