#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MeteorCaptureState;
class QCamera;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0, MeteorCaptureState * state = 0);
private:
    MeteorCaptureState * state;
    QCamera *camera;
    void closeEvent(QCloseEvent *bar);
signals:

public slots:
    void slotInit();
};

#endif // MAINWINDOW_H
