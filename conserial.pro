# This is Qt5 project! It will not work with Qt4
QT       += core gui widgets
# This project uses C+11 features! It requires GCC 4.7
QMAKE_CXXFLAGS += -std=c++11

include(qextserialport/src/qextserialport.pri)

TARGET = conserial
TEMPLATE = app

SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui
