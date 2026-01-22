QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 定义目标名称
TARGET = WeChatStyle
TEMPLATE = app

# 源文件
SOURCES += \
    src/main.cpp \
    src/chatmodel.cpp \
    src/chatdelegate.cpp \
    src/mainwindow.cpp

# 头文件
HEADERS += \
    src/chatmodel.h \
    src/chatdelegate.h \
    src/mainwindow.h

# 针对高分屏的一些默认适配（可选）
DEFINES += QT_DEPRECATED_WARNINGS
