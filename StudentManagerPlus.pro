QT       += core gui sql network charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    databasemanager.cpp \
    finanacialwidget.cpp \
    honorwallwidget.cpp \
    logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    schedulewidget.cpp \
    settings.cpp \
    studentinfowidget.cpp \
    systemsettingswidget.cpp

HEADERS += \
    databasemanager.h \
    finanacialwidget.h \
    honorwallwidget.h \
    logindialog.h \
    mainwindow.h \
    schedulewidget.h \
    settings.h \
    studentinfowidget.h \
    systemsettingswidget.h

FORMS += \
    finanacialwidget.ui \
    honorwallwidget.ui \
    logindialog.ui \
    mainwindow.ui \
    schedulewidget.ui \
    studentinfowidget.ui \
    systemsettingswidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
