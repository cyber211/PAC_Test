#-------------------------------------------------
#
# Project created by QtCreator 2016-10-19T14:07:40
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PAC_Test
TEMPLATE = app
CONFIG   += console  # 程序运行时，带控制台窗口，可以观察程序运行情况；

include(./qextserialport/qextserialport.pri)
include(./xlsx/qtxlsx.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    serialreadingpollingthread.cpp

HEADERS  += mainwindow.h \
    typedefs.h \
    serialreadingpollingthread.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc


VERSION = 0.0.25
QMAKE_TARGET_PRODUCT = PAC TESTER
QMAKE_TARGET_COMPANY = Fortive/Fluke
QMAKE_TARGET_DESCRIPTION = Bob Cao
QMAKE_TARGET_COPYRIGHT = Fluke, All copyright reserved.
