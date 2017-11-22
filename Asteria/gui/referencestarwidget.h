#ifndef REFERENCESTARWIDGET_H
#define REFERENCESTARWIDGET_H

#include "infra/imageuc.h"
#include "infra/source.h"

#include <QWidget>
#include <QMouseEvent>
#include <QWheelEvent>

class AsteriaState;
class GLMeteorDrawer;
class QGroupBox;
class ReferenceStar;

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
    std::shared_ptr<Imageuc> image;

    /**
     * @brief Image viewer for the median image.
     */
    GLMeteorDrawer * signalImageViewer;

    /**
     * @brief Vector of ReferenceStars currently visible.
     */
    std::vector<ReferenceStar *> visibleReferenceStars;

    /**
     * @brief Pointer to the currently selected ReferenceStar.
     */
    ReferenceStar * selectedRefStar;

    /**
     * @brief QGroupBox to contain the reference stars faint magnitude limit slider.
     */
    QGroupBox *refStarMagSliderGroupBox;

    /**
     * @brief Vector of Sources extracted from the calibration image.
     */
    std::vector<Source> sources;

    /**
     * @brief Previous i coordinate of the mouse pointer, for handling drag events.
     */
    int mousePrevI;

    /**
     * @brief Previous j coordinate of the mouse pointer, for handling drag events.
     */
    int mousePrevJ;

    /**
     * @brief I coordinate of mouse pointer on button press; for detecting clicks with no drag motion.
     */
    int mouseStartI;

    /**
     * @brief J coordinate of mouse pointer on button press; for detecting clicks with no drag motion.
     */
    int mouseStartJ;

    /**
     * @brief Recrods current pressed/unpressed state of the left mouse button
     */
    bool leftButtonIsPressed;

    /**
     * @brief Recrods current pressed/unpressed state of the middle mouse button
     */
    bool middleButtonIsPressed;

    /**
     * @brief Recrods current pressed/unpressed state of the right mouse button
     */
    bool rightButtonIsPressed;

    /**
     * @brief Flag used to toggle visualisation of the reference stars.
     */
    bool displayRefStars;

    /**
     * @brief Flag used to toggle visualisation of the extracted sources.
     */
    bool displaySources;

    /**
     * @brief Flag used to toggle visualisation of the geometric calibration.
     */
    bool displayGeoCal;

signals:

public slots:

    void slide(double position);
    void loadImage(std::shared_ptr<Imageuc> &newImage, std::vector<Source> &newSources);
    void update();
    void toggleDisplayRefStars(int checkBoxState);
    void toggleDisplaySources(int checkBoxState);
    void toggleDisplayGeoCal(int checkBoxState);

protected:

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent * event) override;
    void wheelEvent(QWheelEvent * event) override;

};

#endif // REFERENCESTARWIDGET_H
