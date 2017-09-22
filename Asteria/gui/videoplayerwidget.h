#ifndef VIDEOPLAYERWIDGET_H
#define VIDEOPLAYERWIDGET_H

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

class VideoPlayerWidget : public QWidget {

    Q_OBJECT

public:

    explicit VideoPlayerWidget(QWidget *parent, const unsigned int &width, const unsigned int &height, const unsigned int &framePeriodUs);

    GLMeteorDrawer * display;

    ReplayVideoThread * replayThread;

    QPushButton *play_button;
    QPushButton *stop_button;
    QPushButton *pause_button;
    QPushButton *stepf_button;
    QPushButton *stepb_button;
    QSlider * slider;

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
    void loadClip(std::vector<std::shared_ptr<Image>> images, std::shared_ptr<Image> splash);
    void updateVideoStats(const AnalysisVideoStats &stats);

private slots:
};

#endif // VIDEOPLAYERWIDGET_H
