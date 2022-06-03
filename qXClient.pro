QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += . \
               $$PWD

CONFIG += c++17
CONFIG += static

include(sasl/sasl.pri)
include(xmpp/xmpp.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    accountdialog.cpp \
    chatwidget.cpp \
    contactswidget.cpp \
    main.cpp \
    mainwindow.cpp \
    strswitch.cpp \
    tabholder.cpp \
    utils.cpp

HEADERS += \
    accountdialog.h \
    chatwidget.h \
    contactswidget.h \
    mainwindow.h \
    strswitch.h \
    tabholder.h \
    utils.h

FORMS += \
    accountdialog.ui \
    chatwidget.ui \
    contactswidget.ui \
    loginwidget.ui \
    mainwindow.ui \
    tabholder.ui

TRANSLATIONS += \
    qXClient_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
