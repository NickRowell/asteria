#ifndef VIDEODIRNAVIGATOR_H
#define VIDEODIRNAVIGATOR_H

#include <QTreeView>

class MeteorCaptureState;

class VideoDirNavigator : public QTreeView
{
    Q_OBJECT

public:
    VideoDirNavigator(MeteorCaptureState *state = 0);

private:

    // The MeteorCaptureState object
    MeteorCaptureState * state;

};

#endif // VIDEODIRNAVIGATOR_H
