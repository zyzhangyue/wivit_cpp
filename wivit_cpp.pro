TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    csireader.cpp \
    csiwindow.cpp \
    module_modify_csi_power.cpp \
    module_doppler_smooth_csi_mulpal.cpp

QMAKE_LFLAGS += -pthread

LIBS += /usr/lib/x86_64-linux-gnu/libarmadillo.so

HEADERS += \
    csireader.h \
    csi_packet.h \
    csiwindow.h \
    module_modify_csi_power.h \
    module_doppler_smooth_csi_mulpal.h
