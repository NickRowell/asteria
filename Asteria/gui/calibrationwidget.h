#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#define RECALIBRATE

#include "infra/image.h"

#include <QWidget>

class AsteriaState;
class QTreeView;
class VideoDirectoryModel;
class CalibrationInventory;
class VideoPlayerWidget;
#ifdef RECALIBRATE
    class QPushButton;
#endif

class CalibrationWidget : public QWidget {

    Q_OBJECT

public:
    explicit CalibrationWidget(QWidget *parent = 0, AsteriaState * state = 0);

    AsteriaState * state;

    QTreeView *tree;

    VideoDirectoryModel *model;

    /**
     * @brief The CalibrationInventory that is currently being displayed by the widget.
     */
    CalibrationInventory * inv;

    /**
     * @brief Video player widget used to replay the raw calibration footage.
     */
    VideoPlayerWidget * player;

#ifdef RECALIBRATE
    QPushButton *recalibrate_button;
#endif

signals:

public slots:
    // Load a clip for display
    void loadClip(const QModelIndex &index);
    void loadClip(QString path);
    void onCustomContextMenu(const QPoint &point);

#ifdef RECALIBRATE
    void recalibrate();
    void recalibrationComplete(std::string);
#endif

private slots:
};

#endif // CALIBRATIONWIDGET_H
