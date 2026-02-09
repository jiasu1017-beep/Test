QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OfficeAssistant
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           database.cpp \
           appmanagerwidget.cpp \
           fishmodewidget.cpp \
           shutdownwidget.cpp \
           settingswidget.cpp \
           collectionmanagerwidget.cpp

HEADERS  += mainwindow.h \
            database.h \
            appmanagerwidget.h \
            fishmodewidget.h \
            shutdownwidget.h \
            settingswidget.h \
            collectionmanagerwidget.h

FORMS += ui/appmanagerwidget.ui \
         ui/fishmodewidget.ui \
         ui/shutdownwidget.ui \
         ui/settingswidget.ui \
         ui/mainwindow.ui

RESOURCES += resources.qrc
