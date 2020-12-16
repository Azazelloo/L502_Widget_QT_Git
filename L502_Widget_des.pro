QT       += core gui
QT       += network
QT       += widgets serialport
QT       += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    10A.cpp \
    ChangeSampleRate.cpp \
    Generators.cpp \
    L502.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ChangeSampleRate.h \
    Generators.h \
    L502.h \
    inc/l502api.h \
    inc/x502api.h \
    inc/e502api.h \
    inc/l502api.h \
    init10A.h \
    mainwindow.h

#LIBS += \
#    libe502api.a \
#    libl502api.a \
#    libx502api.a


FORMS += \
    ChangeSampleRateDialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/inc/ -le502api
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/inc/ -le502api

INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/inc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/inc/ -ll502api
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/inc/ -ll502api

INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/inc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/inc/ -lx502api
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/inc/ -lx502api
else:unix: LIBS += -L$$PWD/inc/ -lx502api

INCLUDEPATH += $$PWD/inc
DEPENDPATH += $$PWD/inc
