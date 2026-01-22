CHATWIDGET_DIR = $$PWD
MD4C_DIR = $$CHATWIDGET_DIR/../../3rdparty/md4c

INCLUDEPATH += \
    $$CHATWIDGET_DIR \
    $$MD4C_DIR

SOURCES += \
    $$CHATWIDGET_DIR/chatmodel.cpp \
    $$CHATWIDGET_DIR/chatdelegate.cpp \
    $$CHATWIDGET_DIR/qchatwidget.cpp \
    $$CHATWIDGET_DIR/markdownutils.cpp \
    $$MD4C_DIR/md4c.c \
    $$MD4C_DIR/md4c-html.c \
    $$MD4C_DIR/entity.c

HEADERS += \
    $$CHATWIDGET_DIR/chatmodel.h \
    $$CHATWIDGET_DIR/chatdelegate.h \
    $$CHATWIDGET_DIR/qchatwidget.h \
    $$CHATWIDGET_DIR/markdownutils.h \
    $$MD4C_DIR/md4c.h \
    $$MD4C_DIR/md4c-html.h \
    $$MD4C_DIR/entity.h
