QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PonyWork
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           database.cpp \
           appmanagerwidget.cpp \
           fishmodewidget.cpp \
           shutdownwidget.cpp \
           settingswidget.cpp \
           collectionmanagerwidget.cpp \
           recommendedappswidget.cpp

HEADERS  += mainwindow.h \
            database.h \
            appmanagerwidget.h \
            fishmodewidget.h \
            shutdownwidget.h \
            settingswidget.h \
            collectionmanagerwidget.h \
            recommendedappswidget.h

FORMS += ui/appmanagerwidget.ui \
         ui/fishmodewidget.ui \
         ui/shutdownwidget.ui \
         ui/settingswidget.ui \
         ui/mainwindow.ui

RESOURCES += resources.qrc
