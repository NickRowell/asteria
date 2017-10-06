#ifndef REFERENCESTARWIDGET_H
#define REFERENCESTARWIDGET_H

#include "infra/image.h"

#include <QWidget>

class AsteriaState;
class GLMeteorDrawer;
class QPushButton;
class DoubleSlider;

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
     * @brief Up button; used to increase elevation.
     */
    QPushButton *up_button;

    /**
     * @brief Down button; used to decrease elevation.
     */
    QPushButton *down_button;

    /**
     * @brief Left button; used to decrease azimuth.
     */
    QPushButton *left_button;

    /**
     * @brief Right button; used to increase azimuth.
     */
    QPushButton *right_button;

    /**
     * @brief Clockwise button; used to increase roll.
     */
    QPushButton *clockwise_button;

    /**
     * @brief Anticlockwise button; used to decrease roll.
     */
    QPushButton *anticlockwise_button;

    /**
     * @brief Zoom in button; used to increase focal length
     */
    QPushButton *zoomin_button;

    /**
     * @brief Zoom out button; used to deccrease focal length
     */
    QPushButton *zoomout_button;

    /**
     * @brief Slider bar used to adjust the reference star faint magnitude limit.
     */
    DoubleSlider * slider;

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

};

#endif // REFERENCESTARWIDGET_H
