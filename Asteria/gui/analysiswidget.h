#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H

#define REANALYSE

#include "infra/imageuc.h"
#include "gui/videoplayerthread.h"
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
     * The AnalysisInventory that is currently being displayed by the widget.
     * @brief inv
     */
    AnalysisInventory * inv;

    /**
     * @brief Video player widget used to replay the raw calibration footage.
     */
    VideoPlayerWidget * player;

#ifdef REANALYSE
    /**
     * @brief Button to activate the reanalysis of an existing video clip.
     */
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
