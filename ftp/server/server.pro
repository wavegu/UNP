#-------------------------------------------------
#
# Project created by QtCreator 2016-05-02T16:20:44
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    server_main.cpp \
    server_thread.cpp

HEADERS += \
    package.h \
    server_thread.h \
    const.h
