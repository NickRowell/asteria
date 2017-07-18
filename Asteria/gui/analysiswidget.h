#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#include "infra/image.h"
#include "gui/replayvideothread.h"
#include "infra/analysisvideostats.h"

#include <QWidget>

class QPushButton;
class AsteriaState;
class GLMeteorDrawer;
class AnalysisInventory;
class QCheckBox;
class QSlider;
class QLabel;

class AnalysisWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnalysisWidget(QWidget *parent = 0, AsteriaState * state = 0);

    AsteriaState * state;

    /**
     * The AnalysisInventory that is currently being displayed by the widget.
     * @brief inv
     */
    AnalysisInventory * inv;

    GLMeteorDrawer * display;

    ReplayVideoThread * replayThread;

    QPushButton *play_button;
    QPushButton *stop_button;
    QPushButton *pause_button;
    QPushButton *stepf_button;
    QPushButton *stepb_button;
    QSlider * slider;
    QCheckBox *dicheckbox;

    QLabel * clipLengthSecsField;
    QLabel * clipLengthFramesField;
    QLabel * utcField;


signals:

public slots:
    // Load a clip for display
    void loadClip(QString path);
    void updateVideoStats(const AnalysisVideoStats &stats);

private slots:
};

#endif // ANALYSISWIDGET_H
