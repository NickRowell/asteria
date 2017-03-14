#ifndef SYSTEMTAB_H
#define SYSTEMTAB_H

#include <QWidget>

class MeteorCaptureState;

class SystemTab : public QWidget
{
    Q_OBJECT
public:
    explicit SystemTab(MeteorCaptureState * state = 0, QWidget *parent = 0);

signals:

public slots:

private:
    MeteorCaptureState *state;
};

#endif // SYSTEMTAB_H
