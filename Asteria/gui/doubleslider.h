#ifndef DOUBLESLIDER_H
#define DOUBLESLIDER_H

#include <QSlider>

/**
 * @brief The DoubleSlider class is an extension of QSlider that supports floating point numbers.
 */
class DoubleSlider : public QSlider
{

    Q_OBJECT

public:
    /**
     * @brief Main constructor for the DoubleSlider.
     * @param parent
     *  The parent QWidget.
     * @param min
     *  The minimum value of the slider range.
     * @param max
     *  The maximum value of the slider range.
     * @param init
     *  The initial value of the slider.
     * @param steps
     *  The number of discrete steps within the slider range.
     */
    DoubleSlider(QWidget *parent, const double min, const double max, const double init, unsigned int steps);

private:
    /**
     * @brief The minimum value of the slider range.
     */
    double min;
    /**
     * @brief The maximum value of the slider range.
     */
    double max;
    /**
     * @brief The discrete step size of the slider.
     */
    double step;
    /**
     * @brief The number of discrete steps within the slider range.
     */
    int steps;

    /**
     * @brief Transforms the discrete position of the slider to the floating point
     * value within the configured range.
     * @param value
     *  The discrete position of the slider.
     * @return
     *  The equivalent floating point value of the slider position.
     */
    double getDoubleValue(const int value) const;

    /**
     * @brief Transforms a floating point value to the equivalent discrete slider
     * position. Values outside the configured range will be clamped to the boundary.
     * @param value
     *  The floating point value to encode as a discrete slider position.
     * @return
     *  The equivalent discrete slider position.
     */
    int getIntValue(const double value) const;

signals:
    /**
     * @brief Emitted whenever the slider is moved; the signal contains the new floating
     * point value of the slider position.
     * @param value
     *  The new floating point value of the slider position.
     */
    void doubleSliderMoved(double value);

public slots:
    /**
     * @brief Indicates to the DoubleSlider that the slider position has been moved and that
     * the floating point value should be updated.
     * @param value
     *  The discrete position of the slider.
     */
    void notifySliderMoved(int value);
};

#endif // DOUBLESLIDER_H
