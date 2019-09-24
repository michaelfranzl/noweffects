QT       += core gui widgets webkit webkitwidgets

PKGCONFIG += Qt5GStreamer-1.0 Qt5GStreamerUtils-1.0
CONFIG += link_pkgconfig

TARGET = noweffects
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h
