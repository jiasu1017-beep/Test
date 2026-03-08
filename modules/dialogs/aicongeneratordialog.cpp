#include "aicongeneratordialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QPainter>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QBuffer>
#include <QTimer>
#include <QCoreApplication>
#include <QSettings>
#include <cmath>
#include "modules/core/aiconfig.h"

AIIconGeneratorDialog::AIIconGeneratorDialog(QWidget *parent)
    : QDialog(parent), networkManager(new QNetworkAccessManager(this))
{
    setWindowTitle("AI图标生成器");
    setMinimumSize(700, 600);
    resize(800, 700);
    setupUI();
}

AIIconGeneratorDialog::~AIIconGeneratorDialog()
{
}

void AIIconGeneratorDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *methodGroup = new QGroupBox("生成方式", this);
    QVBoxLayout *methodLayout = new QVBoxLayout(methodGroup);

    QHBoxLayout *methodRow = new QHBoxLayout();
    QLabel *methodLabel = new QLabel("选择方式:", this);
    methodCombo = new QComboBox(this);
    methodCombo->addItem("📋 预设模板 (免费)", METHOD_TEMPLATE);
    methodCombo->addItem("🔍 图标库搜索 (免费)", METHOD_ICONFINDER);
    methodCombo->addItem("🎨 硅基流动 (低成本)", METHOD_SILICONFLOW);
    methodCombo->addItem("🌟 OpenAI DALL-E 3 (付费)", METHOD_DALLE3);
    methodCombo->addItem("⚡ Stability AI (付费)", METHOD_STABILITY);
    methodCombo->setCurrentIndex(0);
    connect(methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AIIconGeneratorDialog::onMethodChanged);
    methodRow->addWidget(methodLabel);
    methodRow->addWidget(methodCombo);
    methodLayout->addLayout(methodRow);

    QLabel *methodDesc = new QLabel("💡 推荐优先使用预设模板或图标库搜索，完全免费且速度快", this);
    methodDesc->setStyleSheet("color: #666; font-size: 11px; padding: 5px;");
    methodLayout->addWidget(methodDesc);

    mainLayout->addWidget(methodGroup);

    QGroupBox *configGroup = new QGroupBox("图标配置", this);
    QFormLayout *configLayout = new QFormLayout(configGroup);

    templateCombo = new QComboBox(this);
    templateCombo->addItem("⚙️ 齿轮 - 设置/工具", "gear");
    templateCombo->addItem("📧 信封 - 邮件/消息", "envelope");
    templateCombo->addItem("▶️ 播放 - 视频/音频", "play");
    templateCombo->addItem("⏸️ 暂停 - 暂停/停止", "pause");
    templateCombo->addItem("⏹️ 停止 - 停止/结束", "stop");
    templateCombo->addItem("🔍 搜索 - 搜索/查找", "search");
    templateCombo->addItem("🏠 首页 - 主页/开始", "home");
    templateCombo->addItem("⭐ 收藏 - 收藏/喜欢", "star");
    templateCombo->addItem("📁 文件夹 - 文件夹/目录", "folder");
    templateCombo->addItem("📄 文档 - 文档/文件", "document");
    templateCombo->addItem("🔗 链接 - 链接/网址", "link");
    templateCombo->addItem("📷 相机 - 相机/拍照", "camera");
    templateCombo->addItem("📱 手机 - 手机/电话", "phone");
    templateCombo->addItem("💾 保存 - 保存/存储", "save");
    templateCombo->addItem("🗑️ 删除 - 删除/移除", "trash");
    templateCombo->addItem("✏️ 编辑 - 编辑/修改", "edit");
    templateCombo->addItem("📋 复制 - 复制/克隆", "copy");
    templateCombo->addItem("📌 固定 - 固定/置顶", "pin");
    templateCombo->addItem("🔒 锁定 - 锁定/安全", "lock");
    templateCombo->addItem("🔓 解锁 - 解锁/开放", "unlock");
    templateCombo->addItem("🌐 网络 - 网络/互联网", "network");
    templateCombo->addItem("⚙️ 设置 - 设置/配置", "settings");
    configLayout->addRow("图标类型:", templateCombo);

    colorCombo = new QComboBox(this);
    colorCombo->addItem("🔵 蓝色", "#3498db");
    colorCombo->addItem("🟢 绿色", "#2ecc71");
    colorCombo->addItem("🔴 红色", "#e74c3c");
    colorCombo->addItem("🟡 黄色", "#f1c40f");
    colorCombo->addItem("🟣 紫色", "#9b59b6");
    colorCombo->addItem("🟠 橙色", "#e67e22");
    colorCombo->addItem("⚫ 黑色", "#2c3e50");
    colorCombo->addItem("⚪ 白色", "#ecf0f1");
    colorCombo->addItem("🔵 青色", "#1abc9c");
    colorCombo->addItem("🔵 深蓝", "#2980b9");
    connect(colorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        onColorChanged(colorCombo->itemData(index).toString());
    });
    configLayout->addRow("图标颜色:", colorCombo);

    mainLayout->addWidget(configGroup);

    QGroupBox *promptGroup = new QGroupBox("AI描述 (仅API生成)", this);
    QVBoxLayout *promptLayout = new QVBoxLayout(promptGroup);

    QLabel *promptLabel = new QLabel("图标描述:", this);
    promptLayout->addWidget(promptLabel);

    promptEdit = new QLineEdit(this);
    promptEdit->setPlaceholderText("例如：蓝色的齿轮图标、红色的信封、绿色的播放按钮");
    promptLayout->addWidget(promptEdit);

    QLabel *styleLabel = new QLabel("风格设置 (可选):", this);
    promptLayout->addWidget(styleLabel);

    styleEdit = new QTextEdit(this);
    styleEdit->setMaximumHeight(80);
    styleEdit->setPlaceholderText("可选：minimal design, flat style, gradient, 3D, etc.");
    promptLayout->addWidget(styleEdit);

    mainLayout->addWidget(promptGroup);

    previewLabel = new QLabel(this);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumSize(256, 256);
    previewLabel->setStyleSheet("QLabel { border: 2px dashed #ccc; background: #f5f5f5; border-radius: 8px; }");
    previewLabel->setText("点击生成按钮开始");
    mainLayout->addWidget(previewLabel);

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("padding: 5px; color: #666;");
    mainLayout->addWidget(statusLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    generateBtn = new QPushButton("🎨 生成图标", this);
    generateBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; font-weight: bold; } "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:disabled { background-color: #95a5a6; }"
    );
    connect(generateBtn, &QPushButton::clicked, this, &AIIconGeneratorDialog::onGenerateClicked);

    regenerateBtn = new QPushButton("🔄 重新生成", this);
    regenerateBtn->setEnabled(false);
    regenerateBtn->setStyleSheet(
        "QPushButton { background-color: #e67e22; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; font-weight: bold; } "
        "QPushButton:hover { background-color: #d35400; } "
        "QPushButton:disabled { background-color: #95a5a6; }"
    );
    connect(regenerateBtn, &QPushButton::clicked, this, &AIIconGeneratorDialog::onRegenerateClicked);

    QPushButton *cancelBtn = new QPushButton("取消", this);
    cancelBtn->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; } "
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    okBtn = new QPushButton("✓ 使用此图标", this);
    okBtn->setEnabled(false);
    okBtn->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; font-weight: bold; } "
        "QPushButton:hover { background-color: #229954; } "
        "QPushButton:disabled { background-color: #95a5a6; }"
    );
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addWidget(generateBtn);
    buttonLayout->addWidget(regenerateBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);

    mainLayout->addLayout(buttonLayout);

    onMethodChanged();
}

void AIIconGeneratorDialog::onMethodChanged()
{
    int method = methodCombo->currentData().toInt();
    
    bool isTemplate = (method == METHOD_TEMPLATE);
    bool isIconFinder = (method == METHOD_ICONFINDER);
    bool isAPI = (method == METHOD_SILICONFLOW || method == METHOD_DALLE3 || method == METHOD_STABILITY);
    
    templateCombo->setEnabled(isTemplate);
    colorCombo->setEnabled(isTemplate);
    promptEdit->setEnabled(isIconFinder || isAPI);
    styleEdit->setEnabled(isAPI);
    
    if (isTemplate) {
        promptEdit->setPlaceholderText("使用预设模板生成图标");
        styleEdit->setPlaceholderText("使用预设模板生成图标");
    } else if (method == METHOD_ICONFINDER) {
        promptEdit->setPlaceholderText("输入关键词搜索图标库");
        styleEdit->setPlaceholderText("例如：gear, envelope, play");
    } else {
        promptEdit->setPlaceholderText("例如：蓝色的齿轮图标、红色的信封、绿色的播放按钮");
        styleEdit->setPlaceholderText("可选：minimal design, flat style, gradient, 3D, etc.");
    }
}

void AIIconGeneratorDialog::onColorChanged(const QString &color)
{
    if (methodCombo->currentData().toInt() == METHOD_TEMPLATE) {
        QString iconType = templateCombo->currentData().toString();
        generateTemplateIcon(iconType, color);
    }
}

void AIIconGeneratorDialog::onGenerateClicked()
{
    int method = methodCombo->currentData().toInt();
    
    switch (method) {
        case METHOD_TEMPLATE:
            if (tryTemplateMatch()) {
                return;
            }
            break;
        case METHOD_ICONFINDER:
            if (searchIconLibraries()) {
                return;
            }
            break;
        case METHOD_SILICONFLOW:
            if (useSiliconFlowAPI()) {
                return;
            }
            break;
        case METHOD_DALLE3:
            if (useDALLE3()) {
                return;
            }
            break;
        case METHOD_STABILITY:
            if (useStabilityAPI()) {
                return;
            }
            break;
    }
}

void AIIconGeneratorDialog::onRegenerateClicked()
{
    onGenerateClicked();
}

bool AIIconGeneratorDialog::tryTemplateMatch()
{
    QString iconType = templateCombo->currentData().toString();
    QString color = colorCombo->currentData().toString();
    
    statusLabel->setText("🔄 正在生成预设模板图标...");
    generateTemplateIcon(iconType, color);
    
    return true;
}

void AIIconGeneratorDialog::generateTemplateIcon(const QString &iconType, const QString &color)
{
    QPixmap pixmap(512, 512);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QColor bgColor(color);
    QRadialGradient gradient(256, 256, 256);
    gradient.setColorAt(0, bgColor.lighter(110));
    gradient.setColorAt(0.7, bgColor);
    gradient.setColorAt(1, bgColor.darker(110));
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(50, 50, 412, 412, 100, 100);
    
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    
    int cx = 256;
    int cy = 256;
    
    if (iconType == "gear" || iconType == "settings") {
        painter.drawEllipse(QPointF(cx, cy), 80, 80);
        painter.drawEllipse(QPointF(cx, cy), 30, 30);
        for (int i = 0; i < 8; i++) {
            double angle = i * 45 * 3.1415926 / 180;
            double x1 = cx + cos(angle) * 80;
            double y1 = cy + sin(angle) * 80;
            double x2 = cx + cos(angle) * 110;
            double y2 = cy + sin(angle) * 110;
            painter.drawEllipse(QPointF(x2, y2), 20, 20);
            painter.drawRect(QRectF(x1 - 10, y1 - 10, 20, 20));
        }
    } else if (iconType == "envelope") {
        painter.drawRect(100, 150, 312, 200);
        painter.drawLine(100, 150, 256, 250);
        painter.drawLine(256, 250, 412, 150);
    } else if (iconType == "play") {
        QPolygonF triangle;
        triangle << QPointF(180, 150) << QPointF(180, 362) << QPointF(380, 256);
        painter.drawPolygon(triangle);
    } else if (iconType == "pause") {
        painter.drawRect(150, 150, 60, 212);
        painter.drawRect(302, 150, 60, 212);
    } else if (iconType == "stop") {
        painter.drawRect(130, 130, 252, 252);
    } else if (iconType == "search") {
        painter.drawEllipse(QPointF(220, 220), 70, 70);
        painter.drawLine(270, 270, 350, 350);
    } else if (iconType == "home") {
        QPolygonF roof;
        roof << QPointF(256, 100) << QPointF(120, 220) << QPointF(392, 220);
        painter.drawPolygon(roof);
        painter.drawRect(140, 220, 232, 180);
        painter.drawRect(200, 280, 80, 120);
    } else if (iconType == "star") {
        QPolygonF star;
        for (int i = 0; i < 5; i++) {
            double angle = i * 72 * 3.1415926 / 180 - 90 * 3.1415926 / 180;
            star << QPointF(cx + cos(angle) * 100, cy + sin(angle) * 100);
            double innerAngle = angle + 36 * 3.1415926 / 180;
            star << QPointF(cx + cos(innerAngle) * 40, cy + sin(innerAngle) * 40);
        }
        painter.drawPolygon(star);
    } else if (iconType == "folder") {
        painter.drawRoundedRect(80, 180, 352, 260, 20, 20);
        painter.drawRect(80, 140, 180, 40);
    } else if (iconType == "document") {
        painter.drawRect(150, 100, 212, 312);
        painter.drawLine(150, 160, 362, 160);
        painter.drawLine(150, 220, 362, 220);
        painter.drawLine(150, 280, 362, 280);
        painter.drawLine(150, 340, 300, 340);
    } else if (iconType == "link") {
        painter.drawEllipse(QPointF(200, 200), 60, 60);
        painter.drawEllipse(QPointF(312, 312), 60, 60);
        painter.drawLine(240, 240, 272, 272);
    } else if (iconType == "camera") {
        painter.drawRect(120, 150, 272, 200);
        painter.drawEllipse(QPointF(256, 250), 60, 60);
        painter.drawRect(230, 120, 52, 40);
    } else if (iconType == "phone") {
        painter.drawRoundedRect(190, 100, 132, 312, 20, 20);
        painter.drawRect(230, 380, 52, 20);
    } else if (iconType == "save") {
        painter.drawRect(150, 130, 212, 252);
        painter.drawRect(180, 100, 152, 50);
        painter.drawLine(180, 200, 362, 200);
    } else if (iconType == "trash") {
        painter.drawRect(140, 180, 232, 220);
        painter.drawRect(180, 150, 152, 40);
        painter.drawLine(200, 200, 200, 380);
        painter.drawLine(256, 200, 256, 380);
        painter.drawLine(312, 200, 312, 380);
    } else if (iconType == "edit") {
        painter.drawRect(150, 200, 212, 40);
        QPolygonF pencil;
        pencil << QPointF(380, 100) << QPointF(420, 140) << QPointF(380, 180) << QPointF(340, 140);
        painter.drawPolygon(pencil);
    } else if (iconType == "copy") {
        painter.drawRect(160, 160, 192, 192);
        painter.drawRect(200, 200, 192, 192);
    } else if (iconType == "pin") {
        painter.drawEllipse(QPointF(256, 300), 50, 50);
        painter.drawRect(246, 100, 20, 200);
    } else if (iconType == "lock") {
        painter.drawRect(180, 200, 152, 160);
        painter.drawRect(210, 150, 90, 60);
    } else if (iconType == "unlock") {
        painter.drawRect(180, 200, 152, 160);
        painter.drawArc(210, 130, 90, 60, 0, 180 * 16);
    } else if (iconType == "network") {
        painter.drawEllipse(QPointF(256, 150), 40, 40);
        painter.drawEllipse(QPointF(150, 350), 40, 40);
        painter.drawEllipse(QPointF(362, 350), 40, 40);
        painter.drawLine(256, 190, 150, 310);
        painter.drawLine(256, 190, 362, 310);
        painter.drawLine(190, 350, 322, 350);
    } else {
        painter.drawEllipse(QPointF(cx, cy), 80, 80);
    }
    
    painter.end();
    
    previewLabel->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    statusLabel->setText("✅ 预设模板图标生成成功！");
    
    m_lastImageData.clear();
    QBuffer buffer(&m_lastImageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    
    regenerateBtn->setEnabled(true);
    
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_generatedIconPath = tempPath + "/ai_icon_temp.png";
    QFile file(m_generatedIconPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(m_lastImageData);
        file.close();
    }
    
    regenerateBtn->setEnabled(true);
    generateBtn->setEnabled(true);
    okBtn->setEnabled(true);
}

bool AIIconGeneratorDialog::searchIconLibraries()
{
    QString keyword = promptEdit->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入搜索关键词");
        return false;
    }
    
    bool hasChinese = false;
    for (const QChar &ch : keyword) {
        if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) {
            hasChinese = true;
            break;
        }
    }
    
    if (hasChinese) {
        QMessageBox::information(
            this,
            "关键词提示",
            "本地图标库搜索支持中文和英文关键词。\n\n"
            "示例：\n"
            "• 远程 -> remote, 远程\n"
            "• 计算机 -> computer, 计算机\n"
            "• 齿轮 -> gear, 齿轮\n"
            "• 邮件 -> mail, 邮件\n"
            "• 播放 -> play, 播放\n\n"
            "正在搜索本地图标库..."
        );
    }
    
    statusLabel->setText("🔍 正在搜索本地图标库...");
    generateBtn->setEnabled(false);
    
    QString appDir = QCoreApplication::applicationDirPath();
    QString iconsFolder = appDir + "/app_icons";
    
    QDir dir(iconsFolder);
    if (!dir.exists()) {
        statusLabel->setText("❌ 图标库目录不存在");
        generateBtn->setEnabled(true);
        return false;
    }
    
    QStringList foundIcons;
    
    QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &subDir : subDirs) {
        QDir categoryDir(subDir.absoluteFilePath());
        QStringList filters;
        filters << "*.svg" << "*.png" << "*.ico";
        QFileInfoList iconFiles = categoryDir.entryInfoList(filters, QDir::Files);
        
        for (const QFileInfo &iconFile : iconFiles) {
            QString fileName = iconFile.baseName().toLower();
            QString keywordLower = keyword.toLower();
            
            if (fileName.contains(keywordLower)) {
                foundIcons.append(iconFile.absoluteFilePath());
            }
        }
    }
    
    if (foundIcons.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "未找到匹配图标",
            QString("未找到包含关键词 \"%1\" 的图标。\n\n"
                    "本地图标库包含以下分类：\n"
                    "• 社交媒体\n"
                    "• 办公软件\n"
                    "• 工具类\n"
                    "• 娱乐类\n\n"
                    "您可以：\n"
                    "• 尝试其他关键词\n"
                    "• 使用预设模板（完全免费）\n"
                    "• 使用AI生成功能").arg(keyword),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
        );
        
        if (reply == QMessageBox::Yes) {
            methodCombo->setCurrentIndex(0);
            onMethodChanged();
            statusLabel->setText("ℹ️ 已切换到预设模板模式");
        } else {
            statusLabel->setText("❌ 未找到匹配的图标");
        }
        generateBtn->setEnabled(true);
        return false;
    }
    
    if (foundIcons.size() == 1) {
        QString iconPath = foundIcons.first();
        QPixmap pixmap(iconPath);
        if (pixmap.isNull()) {
            statusLabel->setText("❌ 无法加载图标文件");
            generateBtn->setEnabled(true);
            return false;
        }
        
        previewLabel->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ 找到匹配的图标！");
        
        m_lastImageData.clear();
        QBuffer buffer(&m_lastImageData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m_generatedIconPath = tempPath + "/ai_icon_temp.png";
        QFile file(m_generatedIconPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_lastImageData);
            file.close();
        }
        
        regenerateBtn->setEnabled(true);
        generateBtn->setEnabled(true);
        okBtn->setEnabled(true);
        
        return true;
    } else {
        QString message = QString("找到 %1 个匹配的图标：\n\n").arg(foundIcons.size());
        for (int i = 0; i < qMin(5, foundIcons.size()); ++i) {
            message += QString("• %1\n").arg(QFileInfo(foundIcons[i]).fileName());
        }
        if (foundIcons.size() > 5) {
            message += QString("... 还有 %1 个图标\n").arg(foundIcons.size() - 5);
        }
        message += "\n已使用第一个匹配的图标。";
        
        QMessageBox::information(this, "搜索结果", message);
        
        QString iconPath = foundIcons.first();
        QPixmap pixmap(iconPath);
        if (pixmap.isNull()) {
            statusLabel->setText("❌ 无法加载图标文件");
            generateBtn->setEnabled(true);
            return false;
        }
        
        previewLabel->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ 找到匹配的图标！");
        
        m_lastImageData.clear();
        QBuffer buffer(&m_lastImageData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        
        QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m_generatedIconPath = tempPath + "/ai_icon_temp.png";
        QFile file(m_generatedIconPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_lastImageData);
            file.close();
        }
        
        regenerateBtn->setEnabled(true);
        generateBtn->setEnabled(true);
        okBtn->setEnabled(true);
        
        return true;
    }
}

bool AIIconGeneratorDialog::useSiliconFlowAPI()
{
    QSettings settings("PonyWork", "WorkLog");
    QString encryptedKey = settings.value("ai_image_api_key", "").toString();
    
    if (encryptedKey.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在设置中配置硅基流动API Key\n\n获取地址: https://siliconflow.cn");
        return false;
    }
    
    QByteArray key = QCryptographicHash::hash(QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()), QCryptographicHash::Sha256);
    QByteArray data = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray decrypted;
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    QString apiKey = QString::fromUtf8(decrypted);
    
    QString endpoint = settings.value("ai_image_endpoint", "").toString();
    
    QString prompt = promptEdit->text().trimmed();
    if (prompt.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入图标描述");
        return false;
    }
    
    m_lastPrompt = prompt;
    if (!styleEdit->toPlainText().isEmpty()) {
        m_lastPrompt += ", " + styleEdit->toPlainText();
    }
    m_lastPrompt += ", app icon, minimal design, 512x512";
    
    QString actualEndpoint = endpoint.isEmpty() ? "https://api.siliconflow.cn/v1/images/generations" : endpoint;
    callImageAPI("siliconflow", "black-forest-labs/FLUX.1-schnell", m_lastPrompt, actualEndpoint, apiKey);
    return true;
}

bool AIIconGeneratorDialog::useDALLE3()
{
    QSettings settings("PonyWork", "WorkLog");
    QString encryptedKey = settings.value("ai_image_api_key", "").toString();
    
    if (encryptedKey.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在设置中配置OpenAI API Key\n\n获取地址: https://platform.openai.com");
        return false;
    }
    
    QByteArray key = QCryptographicHash::hash(QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()), QCryptographicHash::Sha256);
    QByteArray data = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray decrypted;
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    QString apiKey = QString::fromUtf8(decrypted);
    
    QString endpoint = settings.value("ai_image_endpoint", "").toString();
    
    QString prompt = promptEdit->text().trimmed();
    if (prompt.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入图标描述");
        return false;
    }
    
    m_lastPrompt = prompt;
    if (!styleEdit->toPlainText().isEmpty()) {
        m_lastPrompt += ", " + styleEdit->toPlainText();
    }
    m_lastPrompt += ", app icon, minimal design, 512x512";
    
    QString actualEndpoint = endpoint.isEmpty() ? "https://api.openai.com/v1/images/generations" : endpoint;
    callImageAPI("openai", "dall-e-3", m_lastPrompt, actualEndpoint, apiKey);
    return true;
}

bool AIIconGeneratorDialog::useStabilityAPI()
{
    QSettings settings("PonyWork", "WorkLog");
    QString encryptedKey = settings.value("ai_image_api_key", "").toString();
    
    if (encryptedKey.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先在设置中配置Stability AI API Key\n\n获取地址: https://platform.stability.ai");
        return false;
    }
    
    QByteArray key = QCryptographicHash::hash(QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()), QCryptographicHash::Sha256);
    QByteArray data = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray decrypted;
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    QString apiKey = QString::fromUtf8(decrypted);
    
    QString endpoint = settings.value("ai_image_endpoint", "").toString();
    
    QString prompt = promptEdit->text().trimmed();
    if (prompt.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入图标描述");
        return false;
    }
    
    m_lastPrompt = prompt;
    if (!styleEdit->toPlainText().isEmpty()) {
        m_lastPrompt += ", " + styleEdit->toPlainText();
    }
    m_lastPrompt += ", app icon, minimal design, 512x512";
    
    QString actualEndpoint = endpoint.isEmpty() ? "https://api.stability.ai/v1/generation/stable-diffusion-xl-1024-v1-0/text-to-image" : endpoint;
    callImageAPI("stability", "stable-diffusion-xl-1024-v1-0", m_lastPrompt, actualEndpoint, apiKey);
    return true;
}

void AIIconGeneratorDialog::callImageAPI(const QString &provider, const QString &model, const QString &prompt, const QString &endpoint, const QString &apiKey)
{
    statusLabel->setText("🔄 正在生成图标...");
    generateBtn->setEnabled(false);
    regenerateBtn->setEnabled(false);
    
    QNetworkRequest request;
    QUrl url(endpoint);
    if (!url.isValid()) {
        statusLabel->setText("❌ API地址无效");
        generateBtn->setEnabled(true);
        return;
    }
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    
    QJsonObject json;
    
    if (provider == "openai") {
        json["model"] = "dall-e-3";
        json["prompt"] = prompt;
        json["n"] = 1;
        json["size"] = "1024x1024";
        json["response_format"] = "url";
    } else if (provider == "siliconflow") {
        json["model"] = model;
        json["prompt"] = prompt;
        json["image_size"] = "1024x1024";
        json["batch_size"] = 1;
        json["num_inference_steps"] = 20;
        json["guidance_scale"] = 7.5;
    } else if (provider == "stability") {
        QJsonObject textPrompts;
        textPrompts["text"] = prompt;
        textPrompts["weight"] = 1;
        QJsonArray promptsArray;
        promptsArray.append(textPrompts);
        
        json["text_prompts"] = promptsArray;
        json["cfg_scale"] = 7;
        json["height"] = 1024;
        json["width"] = 1024;
        json["steps"] = 30;
        json["samples"] = 1;
    }
    
    QJsonDocument doc(json);
    
    QNetworkReply *reply = networkManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, provider]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(data);
            
            QString imageUrl;
            
            if (provider == "openai") {
                QJsonArray dataArray = responseDoc["data"].toArray();
                if (!dataArray.isEmpty()) {
                    imageUrl = dataArray[0].toObject()["url"].toString();
                }
            } else if (provider == "siliconflow") {
                QJsonArray dataArray = responseDoc["images"].toArray();
                if (!dataArray.isEmpty()) {
                    imageUrl = dataArray[0].toObject()["url"].toString();
                }
            } else if (provider == "stability") {
                QJsonArray artifactsArray = responseDoc["artifacts"].toArray();
                if (!artifactsArray.isEmpty()) {
                    QString base64Data = artifactsArray[0].toObject()["base64"].toString();
                    m_lastImageData = QByteArray::fromBase64(base64Data.toUtf8());
                    QPixmap pixmap;
                    pixmap.loadFromData(m_lastImageData);
                    previewLabel->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    statusLabel->setText("✅ 图标生成成功！");
                    saveGeneratedIcon(m_lastImageData);
                    generateBtn->setEnabled(true);
                    regenerateBtn->setEnabled(true);
                    okBtn->setEnabled(true);
                    reply->deleteLater();
                    return;
                }
            }
            
            if (!imageUrl.isEmpty()) {
                downloadImage(imageUrl);
            } else {
                statusLabel->setText("❌ 生成失败: 未返回图片URL");
                generateBtn->setEnabled(true);
            }
        } else {
            QString errorMsg = reply->errorString();
            QByteArray errorData = reply->readAll();
            if (!errorData.isEmpty()) {
                QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
                if (errorDoc.isObject()) {
                    QJsonObject errorObj = errorDoc.object();
                    if (errorObj.contains("error")) {
                        errorMsg = errorObj["error"].toObject()["message"].toString();
                    }
                }
            }
            statusLabel->setText("❌ 生成失败: " + errorMsg);
            generateBtn->setEnabled(true);
        }
        reply->deleteLater();
    });
    
    QTimer::singleShot(60000, this, [this, reply]() {
        if (reply && reply->isRunning()) {
            reply->abort();
            statusLabel->setText("❌ 生成超时");
            generateBtn->setEnabled(true);
        }
    });
}

void AIIconGeneratorDialog::downloadImage(const QString &url)
{
    statusLabel->setText("📥 正在下载图标...");
    
    QNetworkReply *reply = networkManager->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onDownloadFinished(reply);
    });
}

void AIIconGeneratorDialog::onDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        m_lastImageData = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(m_lastImageData);
        
        previewLabel->setPixmap(pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ 图标生成成功！");
        
        saveGeneratedIcon(m_lastImageData);
        
        regenerateBtn->setEnabled(true);
        generateBtn->setEnabled(true);
        okBtn->setEnabled(true);
    } else {
        statusLabel->setText("❌ 下载失败: " + reply->errorString());
        generateBtn->setEnabled(true);
    }
    reply->deleteLater();
}

void AIIconGeneratorDialog::saveGeneratedIcon(const QByteArray &imageData)
{
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString iconsDir = appDataPath + "/custom_icons";
    QDir().mkpath(iconsDir);
    
    QString fileName = QString("ai_icon_%1.png").arg(QDateTime::currentMSecsSinceEpoch());
    QString filePath = iconsDir + "/" + fileName;
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(imageData);
        file.close();
        m_generatedIconPath = filePath;
    }
}

QString AIIconGeneratorDialog::getGeneratedIconPath() const
{
    return m_generatedIconPath;
}
