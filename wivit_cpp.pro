TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    csireader.cpp

QMAKE_LFLAGS += -pthread

LIBS += /usr/lib/x86_64-linux-gnu/libarmadillo.so

HEADERS += \
    csireader.h \
    csi_packet.h
