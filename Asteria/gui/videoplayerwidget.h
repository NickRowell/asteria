#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

#include "infra/image.h"
#include "gui/videoplayerthread.h"
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

class VideoPlayerWidget : public QWidget {

    Q_OBJECT

public:

    explicit VideoPlayerWidget(QWidget *parent, const unsigned int &width, const unsigned int &height, const unsigned int &framePeriodUs);

    /**
     * @brief Used to display the video frames.
     */
    GLMeteorDrawer * display;

    /**
     * @brief Used to load, queue and dispatch the video frames in a dedicated thread.
     */
    VideoPlayerThread * replayThread;

    /**
     * @brief Play button.
     */
    QPushButton *play_button;

    /**
     * @brief Stop button
     */
    QPushButton *stop_button;

    /**
     * @brief Pause button.
     */
    QPushButton *pause_button;

    /**
     * @brief Button used to step forwards one frame at a time.
     */
    QPushButton *stepf_button;

    /**
     * @brief Button used to step backwards one frame at a time.
     */
    QPushButton *stepb_button;

    /**
     * @brief Slider bar used to show progression through the video clip.
     */
    QSlider * slider;

    /**
     * @brief Checkbox for enabling/disabling de-interlaced stepping of frames, for clips composed of interlaced
     * scan type images.
     */
    QCheckBox *dicheckbox;

    /**
     * @brief Checkbox for enabling/disabling display of the overlay image.
     */
    QCheckBox *overlaycheckbox;

    /**
     * @brief Checkbox for enabling/disabling automatic replaying of the video.
     */
    QCheckBox *autoReplayCheckbox;

    /**
     * @brief Label to display the clip length in seconds.
     */
    QLabel * clipLengthSecsField;

    /**
     * @brief Label to display the clip length in frames.
     */
    QLabel * clipLengthFramesField;

    /**
     * @brief Labelto display the UTC of the image currently being displayed.
     */
    QLabel * utcField;

signals:

public slots:
    void loadClip(std::vector<std::shared_ptr<Image>> images, std::shared_ptr<Image> splash);
    void updateVideoStats(const AnalysisVideoStats &stats);

private slots:
};

#endif // VIDEOPLAYERWIDGET_H
