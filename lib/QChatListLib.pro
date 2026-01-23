QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 staticlib
TEMPLATE = lib
TARGET = QChatList

INCLUDEPATH += ../src

include(../src/chatlist/chatlist.pri)

DEFINES += QT_DEPRECATED_WARNINGS

# 可选：make install 时导出头文件/库
HEADERS_INSTALL.files = \
    ../src/chatlist/ChatRoles.h \
    ../src/chatlist/ChatDelegate.h \
    ../src/chatlist/ChatListFilterModel.h \
    ../src/chatlist/ChatListView.h \
    ../src/chatlist/ChatListWidget.h
HEADERS_INSTALL.path = $$OUT_PWD/include

target.path = $$OUT_PWD/lib
INSTALLS += target HEADERS_INSTALL
