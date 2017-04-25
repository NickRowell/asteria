 TEMPLATE = app
TARGET = MeteorCaptureQt

QT = core gui opengl
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    util/IOUtil.cpp \
    util/V4L2Util.cpp \
    gui/cameraselectionwindow.cpp \
    gui/configcreationwindow.cpp \
    gui/mainwindow.cpp \
    gui/glmeteordrawer.cpp \
    util/GLutils.cpp \
    infra/acquisitionthread.cpp \
    util/jpgutil.cpp \
    config/parameterdouble.cpp \
    config/configparameterfamily.cpp \
    config/configstore.cpp \
    gui/configparameterfamilytab.cpp \
    config/parameterdir.cpp \
    infra/image.cpp \
    config/parameteruint.cpp \
    config/parameteruintarray.cpp \
    infra/ringbuffer.cpp \
    util/timeutil.cpp \
    infra/analysisworker.cpp

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
    config/configparameter.h \
    config/configstore.h \
    config/parameterdouble.h \
    config/configparameterfamily.h \
    config/stationparameters.h \
    gui/configparameterfamilytab.h \
    config/systemparameters.h \
    config/cameraparameters.h \
    config/parameterdir.h \
    infra/image.h \
    config/parameteruint.h \
    config/validate.h \
    config/parameteruintarray.h \
    infra/ringbuffer.h \
    config/detectionparameters.h \
    util/timeutil.h \
    infra/analysisworker.h

LIBS += -ljpeg

CONFIG += c++11

DISTFILES += \
    shaders/f_screen.glsl \
    shaders/v_screen.glsl \
    images/side1.png \
    images/side4.png \
    sandbox.txt

RESOURCES += \
    meteorcaptureqt.qrc
