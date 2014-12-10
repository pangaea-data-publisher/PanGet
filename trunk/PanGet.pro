# PanGet - a software tool to download PANGAEA dataset
# Rainer Sieger
# Alfred Wegener Institute, Bremerhaven, Germany
# last change: 2014-10-04

macx {
    message( "MacOS X" )

    # creating cache file
    cache()

    # Set SDK
    QMAKE_MAC_SDK = macosx10.9

    # Only Intel binaries are accepted so force this
    CONFIG += x86_64

    # Minimum OS X version for submission is 10.6.6
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.6

    # Icon has been downloaded from http://www.large-icons.com/stock-icons/free-large-torrent-icons.htm
    ICON = ./Resources/icon/Application.icns

    # Replace default Info.plist
    QMAKE_INFO_PLIST = ./Resources/Info.plist

    QMAKE_CFLAGS += -gdwarf-2
    QMAKE_CXXFLAGS += -gdwarf-2
    QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_OBJECTIVE_CFLAGS_RELEASE =  $$QMAKE_OBJECTIVE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}

win32 {
    message( "Windows" )

    # Icon
    RC_FILE = ./Resources/Application.rc
}

linux-g++ {
    message( "Linux" )
}

TARGET      = PanGet
TEMPLATE    = app

QT += widgets network

INCLUDEPATH = ./Forms

HEADERS     = ./Forms/PanGetDialog.h \
              ./Forms/Webfile.h

FORMS       = ./Forms/pangetdialog.ui

SOURCES     = ./Source/main.cpp \
              ./Forms/Webfile.cpp \
              ./Forms/PanGetDialog.cpp
