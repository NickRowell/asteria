#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class AsteriaState;
class AcquisitionWidget;
class AnalysisWidget;
class CalibrationWidget;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, AsteriaState * state = 0);

private:

    AsteriaState * state;

    AcquisitionWidget * acqWidget;

    AnalysisWidget * analWidget;

    CalibrationWidget * calWidget;

    QTabWidget *tabWidget;

    // Main container widget for all GUI elements
    QWidget * central;

    void closeEvent(QCloseEvent *bar);

signals:

public slots:
    void initAndShowGui();
};

#endif // MAINWINDOW_H
