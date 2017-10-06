#include "doubleslider.h"

DoubleSlider::DoubleSlider(QWidget *parent, const double min, const double max, const double init, unsigned int steps) : QSlider(Qt::Horizontal, parent) {
    this->min = min;
    this->max = max;
    this->steps = steps;
    this->step = (max - min) / steps;

    // Configure the underlying QSlider
    this->setRange(0, steps);
    this->setValue(getIntValue(init));

    // TODO: set tick marks and labels
    this->setSingleStep(1);
    this->setTickPosition(QSlider::TicksBelow);

    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(notifySliderMoved(int)));
}


double DoubleSlider::getDoubleValue(const int value) const {
    return min + value * step;
}

int DoubleSlider::getIntValue(const double value) const {
    if(value < min) {
        return 0;
    }
    else if(value > max) {
        return steps;
    }
    else {
        return (int)std::round((value - min) / step);
    }
}

void DoubleSlider::notifySliderMoved(int value) {
    double doubleValue = getDoubleValue(value);
    emit doubleSliderMoved(doubleValue);
}
