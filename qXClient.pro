QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += . \
               $$PWD

CONFIG += c++17

include(sasl/sasl.pri)
include(xmpp/xmpp.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    accountdialog.cpp \
    accountwidget.cpp \
    chatwidget.cpp \
    contactswidget.cpp \
    dataholder.cpp \
    main.cpp \
    mainwindow.cpp \
    serverinfodialog.cpp \
    strswitch.cpp \
    utils.cpp

HEADERS += \
    accountdialog.h \
    accountwidget.h \
    chatwidget.h \
    contactswidget.h \
    dataholder.h \
    mainwindow.h \
    serverinfodialog.h \
    settings.h \
    strswitch.h \
    utils.h

FORMS += \
    accountdialog.ui \
    accountwidget.ui \
    chatwidget.ui \
    contactswidget.ui \
    loginwidget.ui \
    mainwindow.ui \
    serverinfodialog.ui

TRANSLATIONS += \
    qXClient_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += qdarkstyle/theme/darkstyle.qrc \
  icons/icons.qrc
