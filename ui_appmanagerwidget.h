/********************************************************************************
** Form generated from reading UI file 'appmanagerwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_APPMANAGERWIDGET_H
#define UI_APPMANAGERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AppManagerWidget
{
public:
    QVBoxLayout *mainLayout;
    QLabel *titleLabel;
    QHBoxLayout *buttonLayout;
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *launchButton;
    QPushButton *refreshButton;
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;
    QPushButton *initButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *iconViewButton;
    QPushButton *listViewButton;
    QListView *appListView;

    void setupUi(QWidget *AppManagerWidget)
    {
        if (AppManagerWidget->objectName().isEmpty())
            AppManagerWidget->setObjectName(QString::fromUtf8("AppManagerWidget"));
        AppManagerWidget->resize(800, 600);
        mainLayout = new QVBoxLayout(AppManagerWidget);
        mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
        titleLabel = new QLabel(AppManagerWidget);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setStyleSheet(QString::fromUtf8("font-size: 22px; font-weight: bold; padding: 10px; color: #1976d2;"));

        mainLayout->addWidget(titleLabel);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        addButton = new QPushButton(AppManagerWidget);
        addButton->setObjectName(QString::fromUtf8("addButton"));

        buttonLayout->addWidget(addButton);

        deleteButton = new QPushButton(AppManagerWidget);
        deleteButton->setObjectName(QString::fromUtf8("deleteButton"));

        buttonLayout->addWidget(deleteButton);

        launchButton = new QPushButton(AppManagerWidget);
        launchButton->setObjectName(QString::fromUtf8("launchButton"));

        buttonLayout->addWidget(launchButton);

        refreshButton = new QPushButton(AppManagerWidget);
        refreshButton->setObjectName(QString::fromUtf8("refreshButton"));

        buttonLayout->addWidget(refreshButton);

        moveUpButton = new QPushButton(AppManagerWidget);
        moveUpButton->setObjectName(QString::fromUtf8("moveUpButton"));

        buttonLayout->addWidget(moveUpButton);

        moveDownButton = new QPushButton(AppManagerWidget);
        moveDownButton->setObjectName(QString::fromUtf8("moveDownButton"));

        buttonLayout->addWidget(moveDownButton);

        initButton = new QPushButton(AppManagerWidget);
        initButton->setObjectName(QString::fromUtf8("initButton"));

        buttonLayout->addWidget(initButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(horizontalSpacer);

        iconViewButton = new QPushButton(AppManagerWidget);
        iconViewButton->setObjectName(QString::fromUtf8("iconViewButton"));
        iconViewButton->setCheckable(true);
        iconViewButton->setChecked(true);

        buttonLayout->addWidget(iconViewButton);

        listViewButton = new QPushButton(AppManagerWidget);
        listViewButton->setObjectName(QString::fromUtf8("listViewButton"));
        listViewButton->setCheckable(true);
        listViewButton->setChecked(false);

        buttonLayout->addWidget(listViewButton);


        mainLayout->addLayout(buttonLayout);

        appListView = new QListView(AppManagerWidget);
        appListView->setObjectName(QString::fromUtf8("appListView"));
        appListView->setContextMenuPolicy(Qt::CustomContextMenu);
        appListView->setStyleSheet(QString::fromUtf8("QListView { background-color: #fafafa; border: none; }"));
        appListView->setDragDropMode(QAbstractItemView::NoDragDrop);
        appListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        appListView->setIconSize(QSize(72, 72));
        appListView->setMovement(QListView::Static);
        appListView->setResizeMode(QListView::Adjust);
        appListView->setSpacing(15);
        appListView->setViewMode(QListView::IconMode);

        mainLayout->addWidget(appListView);


        retranslateUi(AppManagerWidget);

        QMetaObject::connectSlotsByName(AppManagerWidget);
    } // setupUi

    void retranslateUi(QWidget *AppManagerWidget)
    {
        AppManagerWidget->setWindowTitle(QCoreApplication::translate("AppManagerWidget", "Form", nullptr));
        titleLabel->setText(QCoreApplication::translate("AppManagerWidget", "<html><head/><body><p>\345\272\224\347\224\250\347\256\241\347\220\206</p></body></html>", nullptr));
        addButton->setText(QCoreApplication::translate("AppManagerWidget", "\346\267\273\345\212\240\345\272\224\347\224\250", nullptr));
        deleteButton->setText(QCoreApplication::translate("AppManagerWidget", "\345\210\240\351\231\244\345\272\224\347\224\250", nullptr));
        launchButton->setText(QCoreApplication::translate("AppManagerWidget", "\345\220\257\345\212\250\345\272\224\347\224\250", nullptr));
        refreshButton->setText(QCoreApplication::translate("AppManagerWidget", "\345\210\267\346\226\260\345\210\227\350\241\250", nullptr));
        moveUpButton->setText(QCoreApplication::translate("AppManagerWidget", "\344\270\212\347\247\273", nullptr));
        moveDownButton->setText(QCoreApplication::translate("AppManagerWidget", "\344\270\213\347\247\273", nullptr));
        initButton->setText(QCoreApplication::translate("AppManagerWidget", "\345\210\235\345\247\213\345\214\226", nullptr));
        iconViewButton->setText(QCoreApplication::translate("AppManagerWidget", "\345\244\247\345\233\276\346\240\207", nullptr));
        listViewButton->setText(QCoreApplication::translate("AppManagerWidget", "\345\210\227\350\241\250", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AppManagerWidget: public Ui_AppManagerWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APPMANAGERWIDGET_H
