MODELCONFIG_DIR = $$PWD

INCLUDEPATH += $$MODELCONFIG_DIR

HEADERS += \
    $$MODELCONFIG_DIR/model_config_import_page.h

SOURCES += \
    $$MODELCONFIG_DIR/model_config_import_page.cpp

!contains(RESOURCES, $$PWD/../../resources/styles.qrc) {
    RESOURCES += $$PWD/../../resources/styles.qrc
}

include($$PWD/../common/qss_utils.pri)
