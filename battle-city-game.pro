TEMPLATE = app

QT += qml quick widgets

CONFIG += c++11

SOURCES += \
    cpp/bcgame.cpp \
    cpp/main.cpp

win32 {
QMAKE_CXXFLAGS = -Wall -Wextra -Werror
QMAKE_CFLAGS = -Wall -Wextra -Werror
}


RESOURCES += qml.qrc \
    qrc/bcgresource.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    cpp/bcgame.h

RC_FILE = application.rc
