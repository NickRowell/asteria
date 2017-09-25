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

    /**
     * @brief Handle to the object storing all state information.
     */
    AsteriaState * state;

    /**
     * @brief Widget to present the video acquisition.
     */
    AcquisitionWidget * acqWidget;

    /**
     * @brief Widget to present the clip analysis.
     */
    AnalysisWidget * analWidget;

    /**
     * @brief Widget to present the camera calibration.
     */
    CalibrationWidget * calWidget;

    /**
     * @brief A tabbed widget used to navigate through the acquisition, analysis and calibration widgets.
     */
    QTabWidget *tabWidget;

    void closeEvent(QCloseEvent *bar);

signals:

public slots:
    void initAndShowGui();
};

#endif // MAINWINDOW_H
