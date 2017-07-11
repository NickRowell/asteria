 TEMPLATE = app
TARGET = Asteria

QT = core gui opengl
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    main.cpp \
    gui/cameraselectionwindow.cpp \
    gui/configcreationwindow.cpp \
    gui/mainwindow.cpp \
    gui/glmeteordrawer.cpp \
    util/jpgutil.cpp \
    config/parameterdouble.cpp \
    config/configparameterfamily.cpp \
    config/configstore.cpp \
    gui/configparameterfamilytab.cpp \
    config/parameterdir.cpp \
    infra/image.cpp \
    config/parameteruint.cpp \
    config/parameteruintarray.cpp \
    util/timeutil.cpp \
    infra/analysisworker.cpp \
    infra/referencestar.cpp \
    infra/acquisitionthread.cpp \
    gui/videodirectorymodel.cpp \
    infra/asteriastate.cpp \
    gui/treeitem.cpp \
    gui/treeitemaction.cpp \
    util/fileutil.cpp \
    gui/acquisitionwidget.cpp \
    gui/analysiswidget.cpp \
    infra/analysisinventory.cpp \
    gui/replayvideothread.cpp \
    infra/calibrationworker.cpp \
    util/ioutil.cpp \
    util/v4l2util.cpp \
    infra/concurrentqueue.cpp \
    infra/videostats.cpp

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
    optics/Camera.h \
    gui/cameraselectionwindow.h \
    gui/configcreationwindow.h \
    gui/mainwindow.h \
    gui/glmeteordrawer.h \
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
    infra/analysisworker.h \
    optics/Camera.h \
    infra/referencestar.h \
    infra/acquisitionthread.h \
    gui/videodirectorymodel.h \
    infra/asteriastate.h \
    gui/treeitem.h \
    gui/treeitemaction.h \
    util/fileutil.h \
    gui/acquisitionwidget.h \
    gui/analysiswidget.h \
    infra/analysisinventory.h \
    gui/replayvideothread.h \
    config/analysisparameters.h \
    config/calibrationparameters.h \
    infra/calibrationworker.h \
    util/ioutil.h \
    util/v4l2util.h \
    infra/concurrentqueue.h \
    infra/videostats.h

LIBS += -ljpeg \
        -lftgl \
        -lfreetype \
        -lGLU
#        -lglut

INCLUDEPATH += /usr/include/freetype2/

CONFIG += c++11

DISTFILES += \
    shaders/f_screen.glsl \
    shaders/v_screen.glsl \
    images/side1.png \
    images/side4.png \
    sandbox.txt

RESOURCES += \
    asteria.qrc
