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
           settingswidget.cpp

HEADERS  += mainwindow.h \
            database.h \
            appmanagerwidget.h \
            fishmodewidget.h \
            shutdownwidget.h \
            settingswidget.h

RESOURCES += resources.qrc
