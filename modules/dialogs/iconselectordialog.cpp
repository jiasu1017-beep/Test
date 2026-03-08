#include "iconselectordialog.h"
#include "aicongeneratordialog.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QIcon>
#include <QPainter>
#include <QFile>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>

IconSelectorDialog::IconSelectorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("选择图标");
    setMinimumSize(650, 500);
    setupUI();
}

IconSelectorDialog::~IconSelectorDialog()
{
}

void IconSelectorDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    tabWidget = new QTabWidget(this);

    socialMediaList = new QListWidget(this);
    socialMediaList->setViewMode(QListWidget::IconMode);
    socialMediaList->setIconSize(QSize(64, 64));
    socialMediaList->setSpacing(15);
    socialMediaList->setMinimumHeight(280);
    socialMediaList->setResizeMode(QListWidget::Adjust);
    socialMediaList->setGridSize(QSize(90, 90));
    socialMediaList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(socialMediaList, &QListWidget::itemDoubleClicked, this, &IconSelectorDialog::onIconDoubleClicked);
    connect(socialMediaList, &QListWidget::itemClicked, this, &IconSelectorDialog::onIconClicked);
    connect(socialMediaList, &QListWidget::customContextMenuRequested, this, &IconSelectorDialog::onIconRightClicked);
    tabWidget->addTab(socialMediaList, "社交媒体");

    officeList = new QListWidget(this);
    officeList->setViewMode(QListWidget::IconMode);
    officeList->setIconSize(QSize(64, 64));
    officeList->setSpacing(15);
    officeList->setMinimumHeight(280);
    officeList->setResizeMode(QListWidget::Adjust);
    officeList->setGridSize(QSize(90, 90));
    officeList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(officeList, &QListWidget::itemDoubleClicked, this, &IconSelectorDialog::onIconDoubleClicked);
    connect(officeList, &QListWidget::itemClicked, this, &IconSelectorDialog::onIconClicked);
    connect(officeList, &QListWidget::customContextMenuRequested, this, &IconSelectorDialog::onIconRightClicked);
    tabWidget->addTab(officeList, "办公软件");

    toolsList = new QListWidget(this);
    toolsList->setViewMode(QListWidget::IconMode);
    toolsList->setIconSize(QSize(64, 64));
    toolsList->setSpacing(15);
    toolsList->setMinimumHeight(280);
    toolsList->setResizeMode(QListWidget::Adjust);
    toolsList->setGridSize(QSize(90, 90));
    toolsList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolsList, &QListWidget::itemDoubleClicked, this, &IconSelectorDialog::onIconDoubleClicked);
    connect(toolsList, &QListWidget::itemClicked, this, &IconSelectorDialog::onIconClicked);
    connect(toolsList, &QListWidget::customContextMenuRequested, this, &IconSelectorDialog::onIconRightClicked);
    tabWidget->addTab(toolsList, "工具类");

    entertainmentList = new QListWidget(this);
    entertainmentList->setViewMode(QListWidget::IconMode);
    entertainmentList->setIconSize(QSize(64, 64));
    entertainmentList->setSpacing(15);
    entertainmentList->setMinimumHeight(280);
    entertainmentList->setResizeMode(QListWidget::Adjust);
    entertainmentList->setGridSize(QSize(90, 90));
    entertainmentList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(entertainmentList, &QListWidget::itemDoubleClicked, this, &IconSelectorDialog::onIconDoubleClicked);
    connect(entertainmentList, &QListWidget::itemClicked, this, &IconSelectorDialog::onIconClicked);
    connect(entertainmentList, &QListWidget::customContextMenuRequested, this, &IconSelectorDialog::onIconRightClicked);
    tabWidget->addTab(entertainmentList, "娱乐类");

    generatedList = new QListWidget(this);
    generatedList->setViewMode(QListWidget::IconMode);
    generatedList->setIconSize(QSize(64, 64));
    generatedList->setSpacing(15);
    generatedList->setMinimumHeight(280);
    generatedList->setResizeMode(QListWidget::Adjust);
    generatedList->setGridSize(QSize(90, 90));
    generatedList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(generatedList, &QListWidget::itemDoubleClicked, this, &IconSelectorDialog::onIconDoubleClicked);
    connect(generatedList, &QListWidget::itemClicked, this, &IconSelectorDialog::onIconClicked);
    connect(generatedList, &QListWidget::customContextMenuRequested, this, &IconSelectorDialog::onIconRightClicked);
    tabWidget->addTab(generatedList, "用户");

    mainLayout->addWidget(tabWidget);

    loadIconsFromAppFolder();
    loadGeneratedIcons();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    browseButton = new QPushButton("浏览自定义图标...", this);
    connect(browseButton, &QPushButton::clicked, this, &IconSelectorDialog::onBrowseCustomIcon);
    buttonLayout->addWidget(browseButton);

    QPushButton *aiGenerateButton = new QPushButton("🤖 AI生成图标...", this);
    //aiGenerateButton->setStyleSheet("QPushButton { background-color: #9b59b6; color: white; padding: 8px 15px; border-radius: 4px; } QPushButton:hover { background-color: #8e44ad; }");
    connect(aiGenerateButton, &QPushButton::clicked, this, &IconSelectorDialog::onAIGenerateIcon);
    buttonLayout->addWidget(aiGenerateButton);
    buttonLayout->addStretch();

    okButton = new QPushButton("确定", this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &IconSelectorDialog::onOkClicked);
    buttonLayout->addWidget(okButton);

    cancelButton = new QPushButton("取消", this);
    connect(cancelButton, &QPushButton::clicked, this, &IconSelectorDialog::onCancelClicked);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);
}

void IconSelectorDialog::loadIconsFromAppFolder()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString iconsFolder = appDir + "/app_icons";

    QDir dir(iconsFolder);
    if (!dir.exists()) {
        loadDefaultIcons();
        return;
    }

    QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    QMap<QString, QListWidget*> categoryMap;
    categoryMap["social_media"] = socialMediaList;
    categoryMap["社交媒体"] = socialMediaList;
    categoryMap["socialmedia"] = socialMediaList;

    categoryMap["office"] = officeList;
    categoryMap["办公软件"] = officeList;
    categoryMap["办公"] = officeList;

    categoryMap["tools"] = toolsList;
    categoryMap["工具类"] = toolsList;
    categoryMap["工具"] = toolsList;

    categoryMap["entertainment"] = entertainmentList;
    categoryMap["娱乐类"] = entertainmentList;
    categoryMap["娱乐"] = entertainmentList;

    for (const QFileInfo &subDirInfo : subDirs) {
        QString dirName = subDirInfo.fileName().toLower();
        QListWidget *targetList = nullptr;

        for (auto it = categoryMap.begin(); it != categoryMap.end(); ++it) {
            if (dirName.contains(it.key().toLower())) {
                targetList = it.value();
                break;
            }
        }

        if (!targetList) {
            if (entertainmentList->count() == 0) {
                targetList = entertainmentList;
            } else {
                targetList = toolsList;
            }
        }

        QDir subDir(subDirInfo.absoluteFilePath());
        QStringList filters;
        filters << "*.svg" << "*.png" << "*.jpg" << "*.jpeg" << "*.ico" << "*.bmp";
        QFileInfoList iconFiles = subDir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo &iconFile : iconFiles) {
            addIconToList(targetList, iconFile.absoluteFilePath(), iconFile.baseName());
        }
    }

    if (socialMediaList->count() == 0 && officeList->count() == 0 &&
        toolsList->count() == 0 && entertainmentList->count() == 0) {
        loadDefaultIcons();
    }
}

void IconSelectorDialog::loadDefaultIcons()
{
    QStringList socialMediaIcons = {
        "facebook.svg|Facebook",
        "instagram.svg|Instagram",
        "youtube.svg|YouTube",
        "whatsapp.svg|WhatsApp",
        "twitter.svg|Twitter/X",
        "tiktok.svg|TikTok",
        "pinterest.svg|Pinterest",
        "snapchat.svg|Snapchat",
        "discord.svg|Discord"
    };
    loadCategoryIcons(socialMediaList, socialMediaIcons);

    QStringList officeIcons = {
        "google.svg|Google",
        "googledocs.svg|Google Docs",
        "googlesheets.svg|Google Sheets",
        "googleslides.svg|Google Slides",
        "icloud.svg|iCloud",
        "notion.svg|Notion",
        "evernote.svg|Evernote",
        "trello.svg|Trello",
        "zoom.svg|Zoom"
    };
    loadCategoryIcons(officeList, officeIcons);

    QStringList toolsIcons = {
        "github.svg|GitHub",
        "git.svg|Git",
        "npm.svg|npm",
        "docker.svg|Docker",
        "figma.svg|Figma",
        "postman.svg|Postman",
        "sketch.svg|Sketch",
        "axios.svg|Axios",
        "android.svg|Android",
        "apple.svg|Apple",
        "linux.svg|Linux",
        "safari.svg|Safari"
    };
    loadCategoryIcons(toolsList, toolsIcons);

    QStringList entertainmentIcons = {
        "spotify.svg|Spotify",
        "netflix.svg|Netflix",
        "twitch.svg|Twitch",
        "steam.svg|Steam",
        "applemusic.svg|Apple Music",
        "youtubemusic.svg|YouTube Music",
        "soundcloud.svg|SoundCloud"
    };
    loadCategoryIcons(entertainmentList, entertainmentIcons);
}

void IconSelectorDialog::loadCategoryIcons(QListWidget *listWidget, const QStringList &icons)
{
    listWidget->clear();
    QString appDir = QCoreApplication::applicationDirPath();

    for (const QString &iconData : icons) {
        QStringList parts = iconData.split("|");
        if (parts.size() != 2) continue;

        QString filename = parts[0];
        QString name = parts[1];

        QString iconPath = appDir + "/icons/" + filename;
        addIconToList(listWidget, iconPath, name);
    }
}

void IconSelectorDialog::loadGeneratedIcons()
{
    generatedList->clear();
    
    QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString iconsDir = userDataDir + "/generated_icons";
    
    QDir dir(iconsDir);
    if (!dir.exists()) {
        return;
    }
    
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.svg";
    QFileInfoList iconFiles = dir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);
    
    for (const QFileInfo &iconFile : iconFiles) {
        QString name = iconFile.baseName();
        QString iconPath = iconFile.absoluteFilePath();
        addIconToList(generatedList, iconPath, name);
    }
}

void IconSelectorDialog::addIconToList(QListWidget *listWidget, const QString &iconPath, const QString &name)
{
    QIcon icon;
    if (QFile::exists(iconPath)) {
        icon.addFile(iconPath, QSize(64, 64));
    } else {
        QPixmap pixmap(64, 64);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setBrush(QColor(70, 130, 180));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(8, 8, 48, 48);
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setBold(true);
        font.setPointSize(20);
        painter.setFont(font);
        painter.drawText(pixmap.rect(), Qt::AlignCenter, name.left(1).toUpper());
        icon.addPixmap(pixmap);
    }

    QListWidgetItem *item = new QListWidgetItem(icon, name, listWidget);
    item->setData(Qt::UserRole, iconPath);
    item->setToolTip(name);
    item->setSizeHint(QSize(90, 90));
}

void IconSelectorDialog::onIconDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        selectedIconPath = item->data(Qt::UserRole).toString();
        accept();
    }
}

void IconSelectorDialog::onBrowseCustomIcon()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        "选择自定义图标",
        QString(),
        "图标文件 (*.ico *.png *.jpg *.bmp *.svg);;所有文件 (*.*)");

    if (!filePath.isEmpty()) {
        selectedIconPath = filePath;
        accept();
    }
}

void IconSelectorDialog::onOkClicked()
{
    QListWidget *currentList = nullptr;
    if (tabWidget->currentWidget() == socialMediaList) currentList = socialMediaList;
    else if (tabWidget->currentWidget() == officeList) currentList = officeList;
    else if (tabWidget->currentWidget() == toolsList) currentList = toolsList;
    else if (tabWidget->currentWidget() == entertainmentList) currentList = entertainmentList;
    else if (tabWidget->currentWidget() == generatedList) currentList = generatedList;

    QListWidgetItem *currentItem = currentList ? currentList->currentItem() : nullptr;

    if (currentItem) {
        selectedIconPath = currentItem->data(Qt::UserRole).toString();
    }

    if (selectedIconPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择一个图标");
        return;
    }

    accept();
}

void IconSelectorDialog::onCancelClicked()
{
    reject();
}

void IconSelectorDialog::onAIGenerateIcon()
{
    AIIconGeneratorDialog aiDialog(this);
    if (aiDialog.exec() == QDialog::Accepted) {
        loadGeneratedIcons();
        tabWidget->setCurrentWidget(generatedList);
    }
}

QString IconSelectorDialog::getSelectedIconPath() const
{
    return selectedIconPath;
}

void IconSelectorDialog::setSelectedIcon(const QString &iconPath)
{
    selectedIconPath = iconPath;
}

void IconSelectorDialog::onIconClicked(QListWidgetItem *item)
{
    if (!item) return;
    currentItem = item;
    QString iconPath = item->data(Qt::UserRole).toString();
    selectedIconPath = iconPath;
}

void IconSelectorDialog::onIconRightClicked(const QPoint &pos)
{
    QListWidget *listWidget = qobject_cast<QListWidget*>(sender());
    if (!listWidget) return;
    
    currentItem = listWidget->itemAt(pos);
    if (!currentItem) return;
    
    QString iconPath = currentItem->data(Qt::UserRole).toString();
    selectedIconPath = iconPath;
    
    contextMenu = new QMenu(this);
    
    deleteAction = new QAction("删除图标", this);
    connect(deleteAction, &QAction::triggered, this, &IconSelectorDialog::onDeleteIcon);
    
    generateFromTemplateAction = new QAction("以此图标为模板生成新图标", this);
    connect(generateFromTemplateAction, &QAction::triggered, this, &IconSelectorDialog::onAIGenerateFromTemplate);
    
    contextMenu->addAction(generateFromTemplateAction);
    contextMenu->addAction(deleteAction);
    
    contextMenu->exec(listWidget->mapToGlobal(pos));
    delete contextMenu;
}

void IconSelectorDialog::onAIGenerateFromTemplate()
{
    if (!currentItem) return;
    
    QString iconPath = currentItem->data(Qt::UserRole).toString();
    
    AIIconGeneratorDialog aiDialog(this);
    aiDialog.setTemplateIcon(iconPath);
    
    if (aiDialog.exec() == QDialog::Accepted) {
        loadGeneratedIcons();
        tabWidget->setCurrentWidget(generatedList);
    }
}

void IconSelectorDialog::onDeleteIcon()
{
    if (!currentItem) return;
    
    QString iconPath = currentItem->data(Qt::UserRole).toString();
    
    int ret = QMessageBox::question(this, "确认删除", 
        "确定要删除这个图标吗？\n\n" + iconPath,
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        QFile file(iconPath);
        if (file.remove()) {
            QListWidget *listWidget = currentItem->listWidget();
            if (listWidget) {
                delete currentItem;
                currentItem = nullptr;
            }
            QMessageBox::information(this, "成功", "图标已删除");
        } else {
            QMessageBox::warning(this, "失败", "删除图标失败");
        }
    }
}

void IconSelectorDialog::onRefreshGeneratedIcons()
{
    loadGeneratedIcons();
}
