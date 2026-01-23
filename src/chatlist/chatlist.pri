CHATLIST_DIR = $$PWD

INCLUDEPATH += $$CHATLIST_DIR

SOURCES += \
    $$CHATLIST_DIR/ChatDelegate.cpp \
    $$CHATLIST_DIR/ChatListFilterModel.cpp \
    $$CHATLIST_DIR/ChatListView.cpp \
    $$CHATLIST_DIR/ChatListWidget.cpp

HEADERS += \
    $$CHATLIST_DIR/ChatRoles.h \
    $$CHATLIST_DIR/ChatDelegate.h \
    $$CHATLIST_DIR/ChatListFilterModel.h \
    $$CHATLIST_DIR/ChatListView.h \
    $$CHATLIST_DIR/ChatListWidget.h
