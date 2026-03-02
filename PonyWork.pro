QT       += core gui network charts printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

win32: LIBS += -lpsapi -lshell32 -lole32 -loleaut32 -lshlwapi -luuid -luser32

TARGET = PonyWork
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           modules/core/database.cpp \
           modules/core/logger.cpp \
           modules/widgets/appmanagerwidget.cpp \
           modules/widgets/fishmodewidget.cpp \
           modules/widgets/shutdownwidget.cpp \
           modules/widgets/settingswidget.cpp \
           modules/widgets/collectionmanagerwidget.cpp \
           modules/widgets/recommendedappswidget.cpp \
           modules/widgets/remotedesktopwidget.cpp \
           modules/widgets/snapshotmanagerwidget.cpp \
           modules/widgets/appcollectionupdater.cpp \
           modules/widgets/worklogwidget.cpp \
           modules/update/updatemanager.cpp \
           modules/update/updatedialog.cpp \
           modules/update/updateprogressdialog.cpp \
           modules/dialogs/desktopsnapshotdialog.cpp \
           modules/dialogs/shortcutdialog.cpp \
           modules/dialogs/iconselectordialog.cpp \
           modules/dialogs/chattestdialog.cpp \
           modules/dialogs/aisettingsdialog.cpp

HEADERS  += mainwindow.h \
            modules/core/database.h \
            modules/core/logger.h \
            modules/widgets/appmanagerwidget.h \
            modules/widgets/fishmodewidget.h \
            modules/widgets/shutdownwidget.h \
            modules/widgets/settingswidget.h \
            modules/widgets/collectionmanagerwidget.h \
            modules/widgets/recommendedappswidget.h \
            modules/widgets/remotedesktopwidget.h \
            modules/widgets/snapshotmanagerwidget.h \
            modules/widgets/appcollectionupdater.h \
            modules/widgets/worklogwidget.h \
            modules/core/appcollectiontypes.h \
            modules/update/updatemanager.h \
            modules/update/updatedialog.h \
            modules/update/updateprogressdialog.h \
            modules/dialogs/desktopsnapshotdialog.h \
            modules/dialogs/shortcutdialog.h \
            modules/dialogs/iconselectordialog.h \
            modules/dialogs/chattestdialog.h \
            modules/dialogs/aisettingsdialog.h \
            modules/core/core.h \
            modules/update/update_module.h \
            modules/widgets/widgets_module.h \
            modules/dialogs/dialogs_module.h

FORMS += modules/ui/appmanagerwidget.ui \
         modules/ui/fishmodewidget.ui \
         modules/ui/shutdownwidget.ui \
         modules/ui/settingswidget.ui \
         modules/ui/mainwindow.ui

RESOURCES += resources.qrc

RC_ICONS = img/icon.ico
