
include(stanza/stanza.pri)
include(stream/stream.pri)

SOURCES += \
		$$PWD/account.cpp \
		$$PWD/addressable.cpp \
		$$PWD/chatchain.cpp \
		$$PWD/contact.cpp \
		$$PWD/contacttreemodel.cpp \
		$$PWD/creds.cpp \
		$$PWD/disco.cpp \
		$$PWD/server.cpp

HEADERS += \
		$$PWD/account.h \
		$$PWD/addressable.h \
		$$PWD/chatchain.h \
		$$PWD/contact.h \
		$$PWD/contacttreemodel.h \
		$$PWD/creds.h \
		$$PWD/disco.h \
		$$PWD/roster.h \
		$$PWD/server.h \
		$$PWD/treeitem.h

RESOURCES +=