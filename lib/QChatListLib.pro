QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 staticlib
TEMPLATE = lib
TARGET = QChatList

INCLUDEPATH += ../src

include(../src/chatlist/chat_list.pri)

DEFINES += QT_DEPRECATED_WARNINGS

# 可选：make install 时导出头文件/库
HEADERS_INSTALL.files = \
    ../src/chatlist/chat_list_roles.h \
    ../src/chatlist/chat_list_delegate.h \
    ../src/chatlist/chat_list_filter_model.h \
    ../src/chatlist/chat_list_view.h \
    ../src/chatlist/chat_list_widget.h
HEADERS_INSTALL.path = $$OUT_PWD/include

target.path = $$OUT_PWD/lib
INSTALLS += target HEADERS_INSTALL
