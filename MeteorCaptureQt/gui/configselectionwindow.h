#ifndef CONFIGSELECTIONWINDOW_H
#define CONFIGSELECTIONWINDOW_H

#include <QWidget>

class QPushButton;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class MeteorCaptureState;

class ConfigSelectionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ConfigSelectionWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);
private:
    MeteorCaptureState * state;
    QPushButton *loadButton;
    QPushButton *createButton;
    QDialogButtonBox *buttonBox;
    QLabel * configLabel;
    QLineEdit * configLine;
signals:
    void ok();
    void cancel();
public slots:

private slots:
    void slotLoadButtonClicked();
    void slotCreateButtonClicked();
    void slotOkButtonClicked();
    void slotCancelButtonClicked();
};

#endif // CONFIGSELECTIONWINDOW_H
