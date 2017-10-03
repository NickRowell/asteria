#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#define RECALIBRATE

#include "infra/image.h"

#include <QWidget>

class ReferenceStarWidget;
class AsteriaState;
class QTreeView;
class VideoDirectoryModel;
class CalibrationInventory;
class VideoPlayerWidget;
class GLMeteorDrawer;
#ifdef RECALIBRATE
    class QPushButton;
#endif

class CalibrationWidget : public QWidget {

    Q_OBJECT

public:
    explicit CalibrationWidget(QWidget *parent = 0, AsteriaState * state = 0);

    /**
     * @brief Handle to the object storing all state information.
     */
    AsteriaState * state;

    /**
     * @brief Tree viewer for video clips that have been captured and analysed.
     */
    QTreeView *tree;

    /**
     * @brief Model of the directory tree containing video clips.
     */
    VideoDirectoryModel *model;

    /**
     * @brief The CalibrationInventory that is currently being displayed by the widget.
     */
    CalibrationInventory * inv;

    /**
     * @brief Video player widget used to replay the raw calibration footage.
     */
    VideoPlayerWidget * player;

    /**
     * @brief Image viewer for the median image.
     */
    ReferenceStarWidget * refStarWidget;

    /**
     * @brief Image viewer for the background image.
     */
    GLMeteorDrawer * backgroundImageViewer;


#ifdef RECALIBRATE
    /**
     * @brief Button to activate the recalibration of existing calibration footage.
     */
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
