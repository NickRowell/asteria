#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#include <QWidget>

class AsteriaState;
class GLMeteorDrawer;

class AnalysisWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnalysisWidget(QWidget *parent = 0, AsteriaState * state = 0);

    AsteriaState * state;
    GLMeteorDrawer * replay;

signals:

public slots:
    // Load a clip for display
    void loadClip(QString path);
};

#endif // ANALYSISWIDGET_H
