#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#define REANALYSE

#include "infra/image.h"
#include "gui/replayvideothread.h"
#include "infra/analysisvideostats.h"

#include <QWidget>

class AsteriaState;
class QTreeView;
class VideoDirectoryModel;
class AnalysisInventory;
class VideoPlayerWidget;
#ifdef REANALYSE
    class QPushButton;
#endif

class AnalysisWidget : public QWidget {

    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget *parent = 0, AsteriaState * state = 0);

    AsteriaState * state;

    QTreeView *tree;

    VideoDirectoryModel *model;

    /**
     * The AnalysisInventory that is currently being displayed by the widget.
     * @brief inv
     */
    AnalysisInventory * inv;

    /**
     * @brief Video player widget used to replay the raw calibration footage.
     */
    VideoPlayerWidget * player;

#ifdef REANALYSE
    QPushButton *reanalyse_button;
#endif

signals:

public slots:
    // Load a clip for display
    void loadClip(const QModelIndex &index);
    void loadClip(QString path);
    void onCustomContextMenu(const QPoint &point);

#ifdef REANALYSE
    void reanalyse();
    void reanalysisComplete(std::string);
#endif

private slots:
};

#endif // ANALYSISWIDGET_H
