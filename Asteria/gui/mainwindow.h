#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class AsteriaState;
class AcquisitionWidget;
class AnalysisWidget;
class QCloseEvent;
class QTreeView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, AsteriaState * state = 0);

private:

    AsteriaState * state;

    AcquisitionWidget * acqWidget;

    AnalysisWidget * analWidget;

    QTreeView *tree;

    QTabWidget *tabWidget;

    // Main container widget for all GUI elements
    QWidget * central;

    void closeEvent(QCloseEvent *bar);

signals:

public slots:
    void initAndShowGui();
    void replayVideo(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &point);
};

#endif // MAINWINDOW_H
