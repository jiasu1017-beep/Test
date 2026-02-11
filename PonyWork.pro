QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32: LIBS += -lpsapi

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
           recommendedappswidget.cpp \
           updatemanager.cpp \
           updatedialog.cpp \
           updateprogressdialog.cpp \
           logger.cpp \
           remotedesktopwidget.cpp \
           desktopsnapshotdialog.cpp

HEADERS  += mainwindow.h \
            database.h \
            appmanagerwidget.h \
            fishmodewidget.h \
            shutdownwidget.h \
            settingswidget.h \
            collectionmanagerwidget.h \
            recommendedappswidget.h \
            updatemanager.h \
            updatedialog.h \
            updateprogressdialog.h \
            logger.h \
            remotedesktopwidget.h \
            desktopsnapshotdialog.h

FORMS += ui/appmanagerwidget.ui \
         ui/fishmodewidget.ui \
         ui/shutdownwidget.ui \
         ui/settingswidget.ui \
         ui/mainwindow.ui

RESOURCES += resources.qrc

RC_ICONS = img/icon.ico
