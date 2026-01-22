QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
INCLUDEPATH += 3rdparty/md4c

# 定义目标名称
TARGET = WeChatStyle
TEMPLATE = app

# 源文件
SOURCES += \
    src/main.cpp \
    src/chatmodel.cpp \
    src/chatdelegate.cpp \
    src/qchatwidget.cpp \
    src/mainwindow.cpp \
    3rdparty/md4c/md4c.c \
    3rdparty/md4c/md4c-html.c \
    3rdparty/md4c/entity.c \
    src/markdownutils.cpp

# 头文件
HEADERS += \
    src/chatmodel.h \
    src/chatdelegate.h \
    src/qchatwidget.h \
    src/mainwindow.h \
    3rdparty/md4c/md4c.h \
    3rdparty/md4c/md4c-html.h \
    3rdparty/md4c/entity.h \
    src/markdownutils.h

# 针对高分屏的一些默认适配（可选）
DEFINES += QT_DEPRECATED_WARNINGS
