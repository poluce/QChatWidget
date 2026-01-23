QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 定义目标名称
TARGET = WeChatStyle
TEMPLATE = app

INCLUDEPATH += ../../src

# 源文件
SOURCES += \
    main.cpp \
    mainwindow.cpp

include(../../src/chatwidget/chat_widget.pri)

# 头文件
HEADERS += \
    mainwindow.h

# 针对高分屏的一些默认适配（可选）
DEFINES += QT_DEPRECATED_WARNINGS
