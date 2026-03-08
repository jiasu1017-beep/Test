#include "aisettingsdialog.h"
#include <QDesktopServices>

AISettingsDialog::AISettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

AISettingsDialog::~AISettingsDialog()
{
}

void AISettingsDialog::setupUI()
{
    setWindowTitle("AI设置指南");
    setMinimumSize(600, 550);
    resize(650, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("🤖 AI配置指南", this);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #333; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    QLabel *descLabel = new QLabel(
        "AI配置功能已移至 <b>设置 → AI设置</b> 中管理。\n"
        "在设置页面中，您可以：\n"
        "• 添加、编辑、删除多个文本AI和图像AI配置\n"
        "• 为不同的AI服务设置API Key\n"
        "• 将常用的AI配置设为默认\n"
        "• 测试AI连接是否正常\n\n"
        "详细配置请前往 <b>设置 → AI设置</b> 页面操作。",
        this
    );
    descLabel->setStyleSheet("padding: 15px; color: #555; font-size: 13px; background-color: #f0f8ff; border-radius: 6px; border: 1px solid #3498db;");
    descLabel->setTextFormat(Qt::RichText);
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);

    QGroupBox *textAiGroup = new QGroupBox("📝 文本AI配置指南", this);
    textAiGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout *textAiLayout = new QVBoxLayout(textAiGroup);

    QTextBrowser *textAiHelpBrowser = new QTextBrowser(this);
    textAiHelpBrowser->setHtml(
        "<div style='padding: 10px; color: #666; font-size: 12px;'>"
        "<b>📖 API Key获取指南:</b><br><br>"
        "• MiniMax: <a href='https://platform.minimaxi.com'>https://platform.minimaxi.com</a><br>"
        "• DeepSeek: <a href='https://siliconflow.cn'>https://siliconflow.cn</a> (推荐，免费100万tokens)<br>"
        "• 通义千问: <a href='https://dashscope.aliyun.com'>https://dashscope.aliyun.com</a><br>"
        "• 讯飞星火: <a href='https://console.xfyun.cn'>https://console.xfyun.cn</a><br>"
        "• OpenAI: <a href='https://platform.openai.com'>https://platform.openai.com</a> (需代理)<br>"
        "• Claude: <a href='https://console.anthropic.com'>https://console.anthropic.com</a> (需代理)<br>"
        "• Gemini: <a href='https://aistudio.google.com/app/apikey'>https://aistudio.google.com/app/apikey</a> (需代理)<br><br>"
        "<b>💡 安全提示:</b> API Key将加密存储在本地配置文件中"
        "</div>"
    );
    textAiHelpBrowser->setStyleSheet("background-color: #f5f5f5; border: none; border-radius: 4px;");
    textAiHelpBrowser->setOpenLinks(false);
    connect(textAiHelpBrowser, &QTextBrowser::anchorClicked, this, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
    textAiLayout->addWidget(textAiHelpBrowser);
    mainLayout->addWidget(textAiGroup);

    QGroupBox *imageAiGroup = new QGroupBox("🎨 图像AI配置指南", this);
    imageAiGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    QVBoxLayout *imageAiLayout = new QVBoxLayout(imageAiGroup);

    QTextBrowser *imageAiHelpBrowser = new QTextBrowser(this);
    imageAiHelpBrowser->setHtml(
        "<div style='padding: 10px; color: #666; font-size: 12px;'>"
        "<b>📖 图像生成API获取指南:</b><br><br>"
        "• 硅基流动: <a href='https://siliconflow.cn'>https://siliconflow.cn</a> (推荐，价格便宜，质量高)<br>"
        "  - Flux Schnell: ¥0.004/张<br>"
        "  - Stable Diffusion 3: ¥0.004/张<br>"
        "• OpenAI: <a href='https://platform.openai.com'>https://platform.openai.com</a> (DALL-E 3, 较贵)<br>"
        "• Stability AI: <a href='https://platform.stability.ai'>https://platform.stability.ai</a> (需代理)<br><br>"
        "<b>💡 推荐使用硅基流动，性价比最高！</b><br><br>"
        "<b>💡 安全提示:</b> API Key将加密存储在本地配置文件中"
        "</div>"
    );
    imageAiHelpBrowser->setStyleSheet("background-color: #f5f5f5; border: none; border-radius: 4px;");
    imageAiHelpBrowser->setOpenLinks(false);
    connect(imageAiHelpBrowser, &QTextBrowser::anchorClicked, this, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });
    imageAiLayout->addWidget(imageAiHelpBrowser);
    mainLayout->addWidget(imageAiGroup);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *goToSettingsBtn = new QPushButton("前往设置", this);
    goToSettingsBtn->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; padding: 10px 25px; border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #2980b9; }"
    );
    connect(goToSettingsBtn, &QPushButton::clicked, this, &QDialog::accept);

    QPushButton *closeBtn = new QPushButton("关闭", this);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; padding: 10px 25px; border-radius: 5px; } "
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    bottomLayout->addStretch();
    bottomLayout->addWidget(goToSettingsBtn);
    bottomLayout->addWidget(closeBtn);
    mainLayout->addLayout(bottomLayout);
}
