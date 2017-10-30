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
    infra/calibrationworker.cpp \
    util/ioutil.cpp \
    util/v4l2util.cpp \
    infra/concurrentqueue.cpp \
    infra/acquisitionvideostats.cpp \
    infra/analysisvideostats.cpp \
    util/mathutil.cpp \
    util/renderutil.cpp \
    math/levenbergmarquardtsolver.cpp \
    math/cosinefitter.cpp \
    math/polynomialfitter.cpp \
    gui/calibrationwidget.cpp \
    infra/calibrationinventory.cpp \
    util/sourcedetector.cpp \
    infra/source.cpp \
    infra/sample.cpp \
    util/coordinateutil.cpp \
    gui/videoplayerwidget.cpp \
    gui/videoplayerthread.cpp \
    util/testutil.cpp \
    config/parameterdoublearray.cpp \
    gui/referencestarwidget.cpp \
    gui/doubleslider.cpp \
    optics/pinholecamera.cpp \
    optics/pinholecamerawithradialdistortion.cpp \
    optics/cameramodelbase.cpp \
    infra/imageuc.cpp \
    infra/meteorimagelocationmeasurement.cpp

HEADERS += \
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
    config/parameteruint.h \
    config/validate.h \
    config/parameteruintarray.h \
    infra/ringbuffer.h \
    config/detectionparameters.h \
    util/timeutil.h \
    infra/analysisworker.h \
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
    config/analysisparameters.h \
    config/calibrationparameters.h \
    infra/calibrationworker.h \
    util/ioutil.h \
    util/v4l2util.h \
    infra/concurrentqueue.h \
    infra/acquisitionvideostats.h \
    infra/analysisvideostats.h \
    util/mathutil.h \
    util/renderutil.h \
    math/levenbergmarquardtsolver.h \
    math/cosinefitter.h \
    math/polynomialfitter.h \
    gui/calibrationwidget.h \
    infra/calibrationinventory.h \
    util/sourcedetector.h \
    infra/source.h \
    infra/sample.h \
    util/coordinateutil.h \
    math/gaussian2dfitter.h \
    gui/videoplayerwidget.h \
    gui/videoplayerthread.h \
    util/testutil.h \
    config/parameterdoublearray.h \
    gui/referencestarwidget.h \
    gui/doubleslider.h \
    optics/pinholecamera.h \
    optics/cameramodelbase.h \
    optics/pinholecamerawithradialdistortion.h \
    infra/imageuc.h \
    infra/image.h \
    infra/meteorimagelocationmeasurement.h

# Add precompiled libraries (-L vs. -l: -L specifies where to look; -l specifies the library name)
LIBS += -L/usr/local/lib -lboost_serialization -lboost_system -lboost_wserialization
LIBS += -ljpeg -lftgl -lfreetype -lGLU

# Includes headers/sources to be compiled into project
INCLUDEPATH += /usr/include/freetype2/ \
               /usr/include/eigen3/ \
               /usr/local/include/boost

CONFIG += c++11

# Compiler optimization results in much faster linear algebra operations
#QMAKE_CXXFLAGS_RELEASE += -O1
#QMAKE_CXXFLAGS_DEBUG += -O1

DISTFILES += \
    images/side1.png \
    images/side4.png \
    sandbox.txt

RESOURCES += \
    asteria.qrc
