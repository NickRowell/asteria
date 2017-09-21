#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#define RECALIBRATE

#include "infra/image.h"
#include "gui/replayvideothread.h"
#include "infra/analysisvideostats.h"

#include <QWidget>

class QPushButton;
class AsteriaState;
class GLMeteorDrawer;
class CalibrationInventory;
class VideoDirectoryModel;
class QCheckBox;
class QSlider;
class QLabel;
class QTreeView;

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

    GLMeteorDrawer * display;

    ReplayVideoThread * replayThread;

    QPushButton *play_button;
    QPushButton *stop_button;
    QPushButton *pause_button;
    QPushButton *stepf_button;
    QPushButton *stepb_button;
    QSlider * slider;

#ifdef RECALIBRATE
    QPushButton *recalibrate_button;
#endif

    /**
     * @brief Checkbox for enabling/disabling de-interlaced stepping of frames, for clips composed of interlaced
     * scan type images.
     */
    QCheckBox *dicheckbox;

    QCheckBox *overlaycheckbox;

    QLabel * clipLengthSecsField;
    QLabel * clipLengthFramesField;
    QLabel * utcField;

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
