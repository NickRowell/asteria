#ifndef REFERENCESTARWIDGET_H
#define REFERENCESTARWIDGET_H

#include "infra/image.h"

#include <QWidget>

class AsteriaState;
class GLMeteorDrawer;
class QGroupBox;

/**
 * @brief Provides a QWidget used to display the median image overlaid with the current positions of
 * the reference stars and with buttons to enable interactive updating of the camera pointing etc.
 */
class ReferenceStarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ReferenceStarWidget(QWidget *parent = 0, AsteriaState * state = 0);

    /**
     * @brief Handle to the object storing all state information.
     */
    AsteriaState * state;

    /**
     * @brief The image currently being displayed.
     */
    std::shared_ptr<Image> image;

    /**
     * @brief Image viewer for the median image.
     */
    GLMeteorDrawer * medianImageViewer;

    /**
     * @brief QGroupBox to contain the reference stars faint magnitude limit slider.
     */
    QGroupBox *refStarMagSliderGroupBox;

signals:

public slots:

    void up();
    void down();
    void left();
    void right();
    void clockwise();
    void anticlockwise();
    void zoomin();
    void zoomout();
    void slide(double position);

    void loadImage(std::shared_ptr<Image> &newImage);

    void update();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // REFERENCESTARWIDGET_H
