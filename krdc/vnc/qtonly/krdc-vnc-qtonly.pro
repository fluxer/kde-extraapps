TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
LIBS += -lvncclient -lgnutls
DEFINES += QTONLY

HEADERS += remoteview.h vncclientthread.h vncview.h
SOURCES += main.cpp remoteview.cpp vncclientthread.cpp vncview.cpp
