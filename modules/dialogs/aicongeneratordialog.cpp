#include "aicongeneratordialog.h"
#include "modules/core/aiconfig.h"
#include "modules/core/ailogger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
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
#include <QTextStream>
#include <QSettings>
#include <QFileInfo>
#include <cmath>

namespace {
    const double PI = 3.14159265358979323846;
    const int ICON_SIZE = 512;
    const int ICON_CENTER = 256;
    const int ICON_RADIUS = 80;
    const int ICON_INNER_RADIUS = 30;
    const int ICON_OUTER_RADIUS = 110;
    const int ICON_TEETH_RADIUS = 20;
    const int ICON_TEETH_SIZE = 20;
    const int PREVIEW_SIZE = 256;
}

AIIconGeneratorDialog::AIIconGeneratorDialog(QWidget *parent)
    : QDialog(parent), networkManager(new QNetworkAccessManager(this)), m_currentReply(nullptr), m_iconSaved(false)
{
    setWindowTitle("AI图标生成器");
    setMinimumSize(900, 800);
    resize(1000, 900);
    setupUI();
}

AIIconGeneratorDialog::~AIIconGeneratorDialog()
{
    if (m_currentReply && m_currentReply->isRunning()) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
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

    QList<AIImageConfig> imageKeys = AIConfig::instance().getAllImageKeys();
    bool hasConfiguredAI = false;
    int defaultIndex = 0;
    int aiStartIndex = methodCombo->count();
    int currentIndex = aiStartIndex;
    for (const AIImageConfig &key : imageKeys) {
        if (!key.apiKey.isEmpty()) {
            hasConfiguredAI = true;
            QString displayText = key.name.isEmpty() ? key.provider : key.name;
            methodCombo->addItem(displayText, key.id);

            if (key.isDefault) {
                defaultIndex = currentIndex;
            }
            currentIndex++;
        }
    }

    if (!hasConfiguredAI) {
        methodCombo->addItem("⚠️ 无可用AI服务", "");
        defaultIndex = 0;
    }

    methodCombo->setCurrentIndex(defaultIndex);

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
    promptEdit->setPlaceholderText("例如：齿轮、信封、播放按钮");
    promptLayout->addWidget(promptEdit);

    QLabel *iconTypeLabel = new QLabel("图标类型:", this);
    promptLayout->addWidget(iconTypeLabel);

    iconTypeCombo = new QComboBox(this);
    iconTypeCombo->addItem("通用图标", "app icon");
    iconTypeCombo->addItem("工具图标", "tool icon");
    iconTypeCombo->addItem("社交图标", "social icon");
    iconTypeCombo->addItem("办公图标", "office icon");
    iconTypeCombo->addItem("娱乐图标", "entertainment icon");
    iconTypeCombo->addItem("设置图标", "settings icon");
    iconTypeCombo->addItem("导航图标", "navigation icon");
    iconTypeCombo->addItem("媒体图标", "media icon");
    promptLayout->addWidget(iconTypeCombo);

    QLabel *designStyleLabel = new QLabel("设计风格:", this);
    promptLayout->addWidget(designStyleLabel);

    designStyleCombo = new QComboBox(this);
    designStyleCombo->addItem("简约设计", "minimal design");
    designStyleCombo->addItem("扁平化", "flat style");
    designStyleCombo->addItem("渐变", "gradient");
    designStyleCombo->addItem("3D效果", "3D");
    designStyleCombo->addItem("线条图标", "line icon");
    designStyleCombo->addItem("填充图标", "filled icon");
    designStyleCombo->addItem("卡通风格", "cartoon style");
    designStyleCombo->addItem("抽象风格", "abstract style");
    designStyleCombo->addItem("现代风格", "modern style");
    designStyleCombo->addItem("复古风格", "retro style");
    promptLayout->addWidget(designStyleCombo);

    QLabel *sizeLabel = new QLabel("图标尺寸:", this);
    promptLayout->addWidget(sizeLabel);

    sizeCombo = new QComboBox(this);
    sizeCombo->addItem("512x512", "512x512");
    sizeCombo->addItem("256x256", "256x256");
    sizeCombo->addItem("1024x1024", "1024x1024");
    sizeCombo->addItem("128x128", "128x128");
    promptLayout->addWidget(sizeCombo);

    QLabel *backgroundLabel = new QLabel("背景:", this);
    promptLayout->addWidget(backgroundLabel);

    backgroundCombo = new QComboBox(this);
    backgroundCombo->addItem("白色背景", "white background");
    backgroundCombo->addItem("透明背景", "transparent background");
    backgroundCombo->addItem("渐变背景", "gradient background");
    backgroundCombo->addItem("纯色背景", "solid color background");
    promptLayout->addWidget(backgroundCombo);

    QLabel *customStyleLabel = new QLabel("自定义描述 (可选):", this);
    promptLayout->addWidget(customStyleLabel);

    styleEdit = new QTextEdit(this);
    styleEdit->setMaximumHeight(60);
    styleEdit->setPlaceholderText("可选：添加其他描述，如颜色、细节等");
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

    okBtn = new QPushButton("使用此图标", this);
    okBtn->setEnabled(false);
    okBtn->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; font-weight: bold; } "
        "QPushButton:hover { background-color: #229954; } "
        "QPushButton:disabled { background-color: #95a5a6; }"
    );
    connect(okBtn, &QPushButton::clicked, this, &AIIconGeneratorDialog::onUseIconClicked);

    saveBtn = new QPushButton("💾 保存图标", this);
    saveBtn->setEnabled(false);
    saveBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; padding: 10px 20px; border-radius: 5px; font-size: 14px; } "
        "QPushButton:hover { background-color: #2980b9; } "
        "QPushButton:disabled { background-color: #95a5a6; }"
    );
    connect(saveBtn, &QPushButton::clicked, this, &AIIconGeneratorDialog::onSaveIconClicked);

    buttonLayout->addWidget(generateBtn);
    buttonLayout->addWidget(regenerateBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);

    mainLayout->addLayout(buttonLayout);

    onMethodChanged();
}

void AIIconGeneratorDialog::onMethodChanged()
{
    QString keyId = methodCombo->currentData().toString();

    if (keyId.isEmpty() || keyId == "-1") {
        QMessageBox::warning(this, "提示", "请先在设置中配置AI图像生成服务");
        methodCombo->setCurrentIndex(0);
        keyId = methodCombo->currentData().toString();
    }

    int method = 0;
    QString provider;
    AIImageConfig keyConfig;
    getMethodAndProviderFromKeyId(keyId, method, provider, keyConfig);

    bool isTemplate = (method == METHOD_TEMPLATE);
    bool isIconFinder = (method == METHOD_ICONFINDER);
    bool isAPI = (method == METHOD_SILICONFLOW || method == METHOD_DALLE3 || method == METHOD_STABILITY);
    
    templateCombo->setEnabled(isTemplate);
    colorCombo->setEnabled(isTemplate);
    promptEdit->setEnabled(isIconFinder || isAPI);
    iconTypeCombo->setEnabled(isAPI);
    designStyleCombo->setEnabled(isAPI);
    sizeCombo->setEnabled(isAPI);
    backgroundCombo->setEnabled(isAPI);
    styleEdit->setEnabled(isAPI);
    
    if (isTemplate) {
        promptEdit->setPlaceholderText("使用预设模板生成图标");
        styleEdit->setPlaceholderText("使用预设模板生成图标");
    } else if (method == METHOD_ICONFINDER) {
        promptEdit->setPlaceholderText("输入关键词搜索图标库");
        styleEdit->setPlaceholderText("例如：gear, envelope, play");
    } else {
        promptEdit->setPlaceholderText("例如：齿轮、信封、播放按钮");
        styleEdit->setPlaceholderText("可选：添加其他描述，如颜色、细节等");
    }
}

void AIIconGeneratorDialog::onColorChanged(const QString &color)
{
    QString keyId = methodCombo->currentData().toString();
    int method = 0;
    QString provider;
    AIImageConfig config;
    getMethodAndProviderFromKeyId(keyId, method, provider, config);

    if (method == METHOD_TEMPLATE) {
        QString iconType = templateCombo->currentData().toString();
        generateTemplateIcon(iconType, color);
    }
}

void AIIconGeneratorDialog::onGenerateClicked()
{
    QString keyId = methodCombo->currentData().toString();

    int method = 0;
    QString provider;
    AIImageConfig keyConfig;
    getMethodAndProviderFromKeyId(keyId, method, provider, keyConfig);

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
        case METHOD_DALLE3:
        case METHOD_STABILITY: {
            if (!keyConfig.id.isEmpty()) {
                QString prompt = promptEdit->text().trimmed();
                if (prompt.isEmpty()) {
                    QMessageBox::warning(this, "提示", "请输入图标描述");
                    return;
                }
                m_lastPrompt = buildPrompt(prompt);
                QString actualEndpoint = keyConfig.endpoint;
                if (actualEndpoint.isEmpty()) {
                    AIImageModelInfo modelInfo = AIConfig::instance().getImageModelInfo(keyConfig.model);
                    actualEndpoint = modelInfo.defaultEndpoint;
                }
                if (actualEndpoint.isEmpty()) {
                    if (provider == "siliconflow") {
                        actualEndpoint = "https://api.siliconflow.cn/v1/images/generations";
                    } else if (provider == "openai") {
                        actualEndpoint = "https://api.openai.com/v1/images/generations";
                    } else if (provider == "stability") {
                        actualEndpoint = "https://api.stability.ai/v1/generation/stable-diffusion-xl-1024-v1-0/text-to-image";
                    }
                }

                callImageAPI(provider, keyConfig.model, m_lastPrompt, actualEndpoint, keyConfig.apiKey);
                return;
            }
            break;
        }
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
    QPixmap pixmap(ICON_SIZE, ICON_SIZE);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    QColor bgColor(color);
    QRadialGradient gradient(ICON_CENTER, ICON_CENTER, ICON_CENTER);
    gradient.setColorAt(0, bgColor.lighter(110));
    gradient.setColorAt(0.7, bgColor);
    gradient.setColorAt(1, bgColor.darker(110));
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(50, 50, 412, 412, 100, 100);
    
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    
    int cx = ICON_CENTER;
    int cy = ICON_CENTER;
    
    if (iconType == "gear" || iconType == "settings") {
        painter.drawEllipse(QPointF(cx, cy), ICON_RADIUS, ICON_RADIUS);
        painter.drawEllipse(QPointF(cx, cy), ICON_INNER_RADIUS, ICON_INNER_RADIUS);
        for (int i = 0; i < 8; i++) {
            double angle = i * 45 * PI / 180;
            double x1 = cx + cos(angle) * ICON_RADIUS;
            double y1 = cy + sin(angle) * ICON_RADIUS;
            double x2 = cx + cos(angle) * ICON_OUTER_RADIUS;
            double y2 = cy + sin(angle) * ICON_OUTER_RADIUS;
            painter.drawEllipse(QPointF(x2, y2), ICON_TEETH_RADIUS, ICON_TEETH_RADIUS);
            painter.drawRect(QRectF(x1 - 10, y1 - 10, ICON_TEETH_SIZE, ICON_TEETH_SIZE));
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
    
    previewLabel->setPixmap(pixmap.scaled(PREVIEW_SIZE, PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    m_lastImageData.clear();
    QBuffer buffer(&m_lastImageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    
    if (saveGeneratedIcon(m_lastImageData)) {
        m_iconSaved = true;
        statusLabel->setText("✅ 预设模板图标生成成功！已自动保存");
    } else {
        m_iconSaved = false;
        statusLabel->setText("✅ 预设模板图标生成成功！但保存失败");
    }
    
    regenerateBtn->setEnabled(true);
    generateBtn->setEnabled(true);
    saveBtn->setEnabled(!m_iconSaved);
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
        
        previewLabel->setPixmap(pixmap.scaled(PREVIEW_SIZE, PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ 找到匹配的图标！");
        
        m_lastImageData.clear();
        QBuffer buffer(&m_lastImageData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        
        if (saveGeneratedIcon(m_lastImageData)) {
            m_iconSaved = true;
            statusLabel->setText("✅ 找到匹配的图标！已自动保存");
        } else {
            m_iconSaved = false;
            statusLabel->setText("✅ 找到匹配的图标！但保存失败");
        }
        
        regenerateBtn->setEnabled(true);
        generateBtn->setEnabled(true);
        saveBtn->setEnabled(!m_iconSaved);
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
        
        previewLabel->setPixmap(pixmap.scaled(PREVIEW_SIZE, PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ 找到匹配的图标！");
        
        m_lastImageData.clear();
        QBuffer buffer(&m_lastImageData);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        
        QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(userDataDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        QString iconsDir = userDataDir + "/generated_icons";
        QDir iconsDirObj(iconsDir);
        if (!iconsDirObj.exists()) {
            iconsDirObj.mkpath(".");
        }
        
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        m_generatedIconPath = iconsDir + "/ai_icon_" + timestamp + ".png";
        QFile file(m_generatedIconPath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(m_lastImageData);
            file.close();
        }
        
        m_iconSaved = false;
        regenerateBtn->setEnabled(true);
        generateBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        okBtn->setEnabled(true);
        
        return true;
    }
}

void AIIconGeneratorDialog::callImageAPI(const QString &provider, const QString &model, const QString &prompt, const QString &endpoint, const QString &apiKey)
{
    statusLabel->setText("🔄 正在生成图标...");
    generateBtn->setEnabled(false);
    regenerateBtn->setEnabled(false);

    m_logFileName = AILogger::initLogFile(AILogger::ImageCall);

    QJsonObject requestJson;
    QString selectedSize = sizeCombo->currentData().toString();

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

    if (provider == "openai") {
        requestJson["model"] = "dall-e-3";
        requestJson["prompt"] = prompt;
        requestJson["n"] = 1;
        requestJson["size"] = selectedSize;
        requestJson["response_format"] = "url";
    } else if (provider == "siliconflow") {
        requestJson["model"] = model;
        requestJson["prompt"] = prompt;
        requestJson["image_size"] = selectedSize;
        requestJson["batch_size"] = 1;
        requestJson["num_inference_steps"] = 20;
        requestJson["guidance_scale"] = 7.5;
    } else if (provider == "stability") {
        QJsonObject textPrompts;
        textPrompts["text"] = prompt;
        textPrompts["weight"] = 1;
        QJsonArray promptsArray;
        promptsArray.append(textPrompts);

        QStringList sizeParts = selectedSize.split("x");
        int width = sizeParts.size() > 0 ? sizeParts[0].toInt() : 1024;
        int height = sizeParts.size() > 1 ? sizeParts[1].toInt() : 1024;

        requestJson["text_prompts"] = promptsArray;
        requestJson["cfg_scale"] = 7;
        requestJson["height"] = height;
        requestJson["width"] = width;
        requestJson["steps"] = 30;
        requestJson["samples"] = 1;
    }

    AILogger::logVerifyRequest(m_logFileName, provider, model, endpoint, requestJson);

    QJsonDocument doc(requestJson);
    m_currentReply = networkManager->post(request, doc.toJson());
    QNetworkReply *reply = m_currentReply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, provider]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument responseDoc = QJsonDocument::fromJson(data);

            int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            AILogger::logVerifyResponse(m_logFileName, httpStatus, responseDoc.object());

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
                    if (!pixmap.loadFromData(m_lastImageData)) {
                        statusLabel->setText("❌ 图标加载失败");
                        generateBtn->setEnabled(true);
                        regenerateBtn->setEnabled(true);
                        reply->deleteLater();
                        m_currentReply = nullptr;
                        return;
                    }
                    previewLabel->setPixmap(pixmap.scaled(PREVIEW_SIZE, PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    statusLabel->setText("✅ 图标生成成功！");
                    AILogger::logVerifySuccess(m_logFileName, "Icon generated and displayed successfully");
                    m_iconSaved = false;
                    
                    // 设置生成的图标路径
                    QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
                    QDir dir(userDataDir);
                    if (!dir.exists()) {
                        dir.mkpath(".");
                    }
                    
                    QString iconsDir = userDataDir + "/generated_icons";
                    QDir iconsDirObj(iconsDir);
                    if (!iconsDirObj.exists()) {
                        iconsDirObj.mkpath(".");
                    }
                    
                    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
                    m_generatedIconPath = iconsDir + "/ai_icon_" + timestamp + ".png";
                    
                    generateBtn->setEnabled(true);
                    regenerateBtn->setEnabled(true);
                    saveBtn->setEnabled(true);
                    okBtn->setEnabled(true);
                    reply->deleteLater();
                    m_currentReply = nullptr;
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

            AILogger::logVerifyError(m_logFileName, errorMsg, QString::fromUtf8(errorData));

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
        m_currentReply = nullptr;
    });
    
    QTimer::singleShot(60000, this, [this]() {
        if (m_currentReply && m_currentReply->isRunning()) {
            m_currentReply->abort();

            AILogger::logVerifyError(m_logFileName, "Request timeout after 60 seconds", "");

            statusLabel->setText("❌ 生成超时");
            generateBtn->setEnabled(true);
            m_currentReply = nullptr;
        }
    });
}

void AIIconGeneratorDialog::downloadImage(const QString &url)
{
    statusLabel->setText("📥 正在下载图标...");
    
    m_currentReply = networkManager->get(QNetworkRequest(QUrl(url)));
    QNetworkReply *reply = m_currentReply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onDownloadFinished(reply);
    });
}

void AIIconGeneratorDialog::onDownloadFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        m_lastImageData = reply->readAll();
        QPixmap pixmap;
        if (!pixmap.loadFromData(m_lastImageData)) {
            statusLabel->setText("❌ 图标加载失败");
            generateBtn->setEnabled(true);
            reply->deleteLater();
            m_currentReply = nullptr;
            return;
        }

        previewLabel->setPixmap(pixmap.scaled(PREVIEW_SIZE, PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("✅ 图标生成成功！");
        AILogger::logVerifySuccess(m_logFileName, "Icon URL retrieved successfully");

        m_iconSaved = false;
        
        // 设置生成的图标路径
        QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(userDataDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        
        QString iconsDir = userDataDir + "/generated_icons";
        QDir iconsDirObj(iconsDir);
        if (!iconsDirObj.exists()) {
            iconsDirObj.mkpath(".");
        }
        
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        m_generatedIconPath = iconsDir + "/ai_icon_" + timestamp + ".png";
        
        regenerateBtn->setEnabled(true);
        generateBtn->setEnabled(true);
        saveBtn->setEnabled(true);
        okBtn->setEnabled(true);
    } else {
        QString errorMsg = reply->errorString();

        AILogger::logVerifyError(m_logFileName, errorMsg, "");

        statusLabel->setText("❌ 下载失败: " + errorMsg);
        generateBtn->setEnabled(true);
    }
    reply->deleteLater();
    m_currentReply = nullptr;
}

bool AIIconGeneratorDialog::saveGeneratedIcon(const QByteArray &imageData)
{
    QString userDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(userDataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QString iconsDir = userDataDir + "/generated_icons";
    QDir iconsDirObj(iconsDir);
    if (!iconsDirObj.exists()) {
        iconsDirObj.mkpath(".");
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    m_generatedIconPath = iconsDir + "/ai_icon_" + timestamp + ".png";
    
    QFile file(m_generatedIconPath);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 bytesWritten = file.write(imageData);
        file.close();
        if (bytesWritten != imageData.size()) {
            QMessageBox::warning(this, "错误", "图标保存失败，请检查磁盘空间");
            m_generatedIconPath.clear();
            return false;
        }
        return true;
    } else {
        QMessageBox::warning(this, "错误", "无法打开文件进行保存");
        m_generatedIconPath.clear();
        return false;
    }
}

void AIIconGeneratorDialog::onSaveIconClicked()
{
    if (m_lastImageData.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有可保存的图标");
        return;
    }
    
    if (saveGeneratedIcon(m_lastImageData)) {
        m_iconSaved = true;
        saveBtn->setEnabled(false);
        QMessageBox::information(this, "成功", "图标已保存到:\n" + m_generatedIconPath);
    } else {
        m_iconSaved = false;
        QMessageBox::warning(this, "错误", "图标保存失败");
    }
}

void AIIconGeneratorDialog::onUseIconClicked()
{
    if (!m_iconSaved && !m_lastImageData.isEmpty()) {
        if (saveGeneratedIcon(m_lastImageData)) {
            m_iconSaved = true;
        } else {
            return;
        }
    }
    
    if (m_generatedIconPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "图标路径无效，无法使用");
        return;
    }
    
    accept();
}

QString AIIconGeneratorDialog::getGeneratedIconPath() const
{
    return m_generatedIconPath;
}

void AIIconGeneratorDialog::setTemplateIcon(const QString &iconPath)
{
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        previewLabel->setPixmap(pixmap.scaled(PREVIEW_SIZE, PREVIEW_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        statusLabel->setText("📋 已选择模板图标");
        
        QFile file(iconPath);
        if (file.open(QIODevice::ReadOnly)) {
            m_lastImageData = file.readAll();
            file.close();
            if (m_lastImageData.isEmpty()) {
                QMessageBox::warning(this, "错误", "无法读取模板图标文件");
                return;
            }
        } else {
            QMessageBox::warning(this, "错误", "无法打开模板图标文件");
            return;
        }
        
        QString fileName = QFileInfo(iconPath).baseName();
        promptEdit->setText(fileName);
        
        methodCombo->setCurrentIndex(METHOD_SILICONFLOW);
    } else {
        QMessageBox::warning(this, "错误", "无法加载模板图标");
    }
}

QString AIIconGeneratorDialog::decryptApiKey(const QString &encryptedKey)
{
    if (encryptedKey.isEmpty()) {
        return QString();
    }
    
    QByteArray key = QCryptographicHash::hash(QByteArray("PonyWorkAI").append(QHostInfo::localHostName().toUtf8()), QCryptographicHash::Sha256);
    QByteArray data = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray decrypted;
    for (int i = 0; i < data.size(); ++i) {
        decrypted.append(data.at(i) ^ key.at(i % key.size()));
    }
    return QString::fromUtf8(decrypted);
}

void AIIconGeneratorDialog::getMethodAndProviderFromKeyId(const QString &keyId, int &method, QString &provider, AIImageConfig &config)
{
    method = 0;
    provider.clear();
    config = AIConfig::instance().getImageKey(keyId);
    if (!config.id.isEmpty()) {
        provider = config.provider;
        if (provider == "siliconflow") method = METHOD_SILICONFLOW;
        else if (provider == "openai") method = METHOD_DALLE3;
        else if (provider == "stability") method = METHOD_STABILITY;
    } else {
        method = keyId.toInt();
    }
}

QString AIIconGeneratorDialog::buildPrompt(const QString &basePrompt)
{
    QString prompt = basePrompt;
    
    QString iconType = iconTypeCombo->currentData().toString();
    QString designStyle = designStyleCombo->currentData().toString();
    QString size = sizeCombo->currentData().toString();
    QString background = backgroundCombo->currentData().toString();
    
    prompt += ", " + iconType;
    prompt += ", " + designStyle;
    prompt += ", " + size;
    prompt += ", " + background;
    
    if (!styleEdit->toPlainText().isEmpty()) {
        prompt += ", " + styleEdit->toPlainText();
    }
    
    return prompt;
}
