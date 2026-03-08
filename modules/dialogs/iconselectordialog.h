#ifndef ICONSELECTORDIALOG_H
#define ICONSELECTORDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QIcon>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QMap>
#include <QStandardPaths>
#include <QMenu>
#include <QAction>

class IconSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IconSelectorDialog(QWidget *parent = nullptr);
    ~IconSelectorDialog();

    QString getSelectedIconPath() const;
    void setSelectedIcon(const QString &iconPath);

private slots:
    void onIconDoubleClicked(QListWidgetItem *item);
    void onIconClicked(QListWidgetItem *item);
    void onIconRightClicked(const QPoint &pos);
    void onBrowseCustomIcon();
    void onAIGenerateIcon();
    void onAIGenerateFromTemplate();
    void onOkClicked();
    void onCancelClicked();
    void onDeleteIcon();
    void onRefreshGeneratedIcons();

private:
    void setupUI();
    void loadIconsFromAppFolder();
    void loadDefaultIcons();
    void loadGeneratedIcons();
    void loadCategoryIcons(QListWidget *listWidget, const QStringList &icons);
    void addIconToList(QListWidget *listWidget, const QString &iconPath, const QString &name);

    QTabWidget *tabWidget;
    QListWidget *socialMediaList;
    QListWidget *officeList;
    QListWidget *toolsList;
    QListWidget *entertainmentList;
    QListWidget *generatedList;
    QLabel *previewLabel;
    QPushButton *browseButton;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QString selectedIconPath;
    QListWidgetItem *currentItem;
    QMenu *contextMenu;
    QAction *deleteAction;
    QAction *generateFromTemplateAction;
};

#endif // ICONSELECTORDIALOG_H
