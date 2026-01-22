QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 staticlib
TEMPLATE = lib
TARGET = QChatWidget

INCLUDEPATH += ../src

include(../src/chatwidget/chatwidget.pri)

DEFINES += QT_DEPRECATED_WARNINGS

# 可选：make install 时导出头文件/库
HEADERS_INSTALL.files = \
    ../src/chatwidget/chatmodel.h \
    ../src/chatwidget/chatdelegate.h \
    ../src/chatwidget/chatviewwidget.h \
    ../src/chatwidget/chatinputwidget.h \
    ../src/chatwidget/qchatwidget.h \
    ../src/chatwidget/markdownutils.h
HEADERS_INSTALL.path = $$OUT_PWD/include

target.path = $$OUT_PWD/lib
INSTALLS += target HEADERS_INSTALL
