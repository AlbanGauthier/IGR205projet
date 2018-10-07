TEMPLATE = app
TARGET = nameOfYourApp
DEPENDPATH += .
INCLUDEPATH += .
INCLUDEPATH += ./src
QT += opengl xml
CONFIG += qt \
    release
CONFIG += c++11
MOC_DIR = ./tmp/moc
OBJECTS_DIR = ./tmp/obj




#############    TODO    ################
# Set extern directory (where you compiled your libraries)
EXT_DIR = /C:/Users/Documents/PathToTheProject/extern



# Input
HEADERS += ./src/point3.h \
    ./src/Mesh.h \
    ./src/MyViewer.h \
    ./src/tetgenhandler.h \
    ./src/gl/GLUtilityMethods.h \
    ./src/gl/BasicColors.h \
    ./src/KDTree.h \
    $${EXT_DIR}/tetgen1.5.1-beta1/tetgen.h \
    src/point4.h
SOURCES += ./src/main.cpp \
    ./src/gl/GLUtilityMethods.cpp\
    ./src/gl/BasicColors.cpp \
    ../extern/tetgen1.5.1-beta1/tetgen.cxx \
    ../extern/tetgen1.5.1-beta1/predicates.cxx


#tetgen
 INCLUDEPATH += ../extern/tetgen1.5.1-beta1


#QGLViewer
{
 INCLUDEPATH += $${EXT_DIR}/libQGLViewer-2.6.1
 LIBS +=    -L$${EXT_DIR}/libQGLViewer-2.6.1/QGLViewer \
            -lQGLViewer
}


#GraphCut
HEADERS += $${EXT_DIR}/max_flow/graph.h
SOURCES += $${EXT_DIR}/max_flow/graph.cpp
SOURCES += $${EXT_DIR}/max_flow/maxflow.cpp
INCLUDEPATH += $${EXT_DIR}/max_flow


LIBS += -lglut \
    -lGLU
LIBS += -lgsl \
    -lgomp
LIBS += -lgslcblas \
    -lgomp
release:QMAKE_CXXFLAGS_RELEASE += -O3 \
    -fopenmp
release:QMAKE_CFLAGS_RELEASE += -O3 \
    -fopenmp
