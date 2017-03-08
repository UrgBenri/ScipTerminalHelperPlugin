!contains( included_modules, $$PWD ) {
    included_modules += $$PWD
    QT += core gui

    !include($$PWD/PluginInterface/UrgBenriPluginInterface.pri) {
            error("Unable to include Viewer Plugin Interface.")
    }

    !include($$PWD/QUrgLib/QUrgLib.pri) {
            error("Unable to include QUrg Library.")
    }

    DEPENDPATH += $$PWD
    INCLUDEPATH += $$PWD

    SOURCES += \
        $$PWD/ScipTerminalHelperPlugin.cpp \
        $$PWD/CommandLine.cpp

    HEADERS  += \
        $$PWD/ScipTerminalHelperPlugin.h \
        $$PWD/CommandLine.h

    FORMS += \
        $$PWD/ScipTerminalHelperPlugin.ui

    RESOURCES += \
        $$PWD/ScipTerminalHelperPlugin.qrc

    TRANSLATIONS = $$PWD/i18n/plugin_fr.ts \
        $$PWD/i18n/plugin_en.ts \
        $$PWD/i18n/plugin_ja.ts
}

