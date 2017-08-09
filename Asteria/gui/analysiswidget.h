#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#define REANALYSE

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

#ifdef REANALYSE
    QPushButton *reanalyse_button;
#endif

    /**
    * @brief dicheckbox
    * Checkbox for enabling/disabling de-interlaced stepping of frames, for clips composed of interlaced
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
    void loadClip(QString path);
    void updateVideoStats(const AnalysisVideoStats &stats);

#ifdef REANALYSE
    void reanalyse();
    void reanalysisComplete(std::string);
#endif

private slots:
};

#endif // ANALYSISWIDGET_H
