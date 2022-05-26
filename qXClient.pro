QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    account.cpp \
    addressable.cpp \
    creds.cpp \
    infoquery.cpp \
    main.cpp \
    mainwindow.cpp \
    saslgenerator.cpp \
    scramgenerator.cpp \
    server.cpp \
    stanza.cpp \
    stream.cpp \
    streamfeature.cpp \
    strswitch.cpp \
    utils.cpp

HEADERS += \
    account.h \
    addressable.h \
    creds.h \
    infoquery.h \
    mainwindow.h \
    saslgenerator.h \
    saslmechanisms.h \
    scramgenerator.h \
    server.h \
    stanza.h \
    stream.h \
    streamfeature.h \
    strswitch.h \
    utils.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    qXClient_en_US.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
