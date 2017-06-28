#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#include "infra/image.h"
#include "gui/replayvideothread.h"

#include <QWidget>

class QButtonGroup;
class QPushButton;
class AsteriaState;
class GLMeteorDrawer;
class AnalysisInventory;
class QCheckBox;
class QSlider;

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

    GLMeteorDrawer * replay;

    ReplayVideoThread * player;

    QPushButton *play_button;
    QPushButton *stop_button;
    QPushButton *pause_button;
    QPushButton *stepf_button;
    QPushButton *stepb_button;
    QSlider * slider;
    QCheckBox *dicheckbox;

signals:

public slots:
    // Load a clip for display
    void loadClip(QString path);

private slots:
};

#endif // ANALYSISWIDGET_H
