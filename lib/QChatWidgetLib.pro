QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 staticlib
TEMPLATE = lib
TARGET = QChatWidget

INCLUDEPATH += ../src

include(../src/chatwidget/chat_widget.pri)

DEFINES += QT_DEPRECATED_WARNINGS

# 可选：make install 时导出头文件/库
HEADERS_INSTALL.files = \
    ../src/chatwidget/chat_widget_model.h \
    ../src/chatwidget/chat_widget_delegate.h \
    ../src/chatwidget/chat_widget_view.h \
    ../src/chatwidget/chat_widget_input.h \
    ../src/chatwidget/q_chat_widget.h \
    ../src/chatwidget/chat_widget_markdown_utils.h
HEADERS_INSTALL.path = $$OUT_PWD/include

target.path = $$OUT_PWD/lib
INSTALLS += target HEADERS_INSTALL
