 TEMPLATE = app
TARGET = MeteorCaptureQt

QT = core gui opengl
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    util/IOUtil.cpp \
    util/V4L2Util.cpp \
    MeteorCapture.cpp \
    gui/cameraselectionwindow.cpp \
    gui/configcreationwindow.cpp \
    gui/mainwindow.cpp \
    gui/glmeteordrawer.cpp \
    util/GLutils.cpp \
    infra/acquisitionthread.cpp \
    util/jpgutil.cpp \
    gui/systemtab.cpp \
    gui/stationtab.cpp \
    gui/cameratab.cpp

HEADERS += \
    math/mat2x2.h \
    math/mat3x3.h \
    math/mat4x4.h \
    math/quat.h \
    math/vec2.h \
    math/vec3.h \
    math/vec4.h \
    optics/GeometricOpticsModel.h \
    optics/PinholeCameraWithRadialDistortion.h \
    util/IOUtil.h \
    util/V4L2Util.h \
    Camera.h \
    gui/cameraselectionwindow.h \
    infra/meteorcapturestate.h \
    gui/configcreationwindow.h \
    gui/mainwindow.h \
    gui/glmeteordrawer.h \
    util/GLutils.h \
    infra/acquisitionthread.h \
    util/jpgutil.h \
    gui/systemtab.h \
    gui/stationtab.h \
    gui/cameratab.h

LIBS += -ljpeg

DISTFILES += \
    shaders/f_screen.glsl \
    shaders/v_screen.glsl \
    images/side1.png \
    images/side4.png

RESOURCES += \
    meteorcaptureqt.qrc
