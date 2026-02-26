#include "shortcutdialog.h"
#include "database.h"
#include <QTimer>

ShortcutDialog::ShortcutDialog(Database *db, QWidget *parent)
    : QDialog(parent), db(db)
{
    setupUI();
    originalShortcut = QKeySequence(db->getShortcutKey());
    shortcutEdit->setKeySequence(originalShortcut);
}

ShortcutDialog::~ShortcutDialog()
{
}

QKeySequence ShortcutDialog::getShortcut() const
{
    return shortcutEdit->keySequence();
}

void ShortcutDialog::setShortcut(const QKeySequence &shortcut)
{
    shortcutEdit->setKeySequence(shortcut);
}

void ShortcutDialog::setupUI()
{
    setWindowTitle("修改全局快捷键");
    setMinimumSize(350, 150);
    setMaximumSize(500, 200);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(10);
    
    QLabel *label = new QLabel("快捷键:", this);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    label->setMinimumWidth(60);
    
    shortcutEdit = new QKeySequenceEdit(this);
    shortcutEdit->setToolTip("点击输入框，然后按下您想要使用的快捷键组合");
    shortcutEdit->setFocusPolicy(Qt::StrongFocus);
    shortcutEdit->setMinimumWidth(200);
    
    inputLayout->addWidget(label);
    inputLayout->addWidget(shortcutEdit, 1);
    
    mainLayout->addLayout(inputLayout);

    statusLabel = new QLabel("请输入快捷键组合", this);
    statusLabel->setStyleSheet("color: #666; font-size: 12px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    QPushButton *resetButton = new QPushButton("重置为默认", this);
    resetButton->setToolTip("将快捷键重置为默认的 Ctrl+W");
    resetButton->setMinimumWidth(100);
    
    okButton = new QPushButton("确定", this);
    okButton->setDefault(true);
    okButton->setMinimumWidth(80);
    
    QPushButton *cancelButton = new QPushButton("取消", this);
    cancelButton->setMinimumWidth(80);
    
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);

    connect(shortcutEdit, &QKeySequenceEdit::keySequenceChanged, this, &ShortcutDialog::onKeySequenceChanged);
    connect(resetButton, &QPushButton::clicked, this, &ShortcutDialog::onResetButtonClicked);
    connect(okButton, &QPushButton::clicked, this, &ShortcutDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ShortcutDialog::onCancelButtonClicked);

    QTimer::singleShot(100, this, SLOT(activateWindow()));
}

void ShortcutDialog::onKeySequenceChanged(const QKeySequence &sequence)
{
    if (sequence.isEmpty()) {
        updateStatusMessage("请输入有效的快捷键组合", true);
        okButton->setEnabled(false);
        return;
    }

    QString shortcutStr = sequence.toString();

    if (isShortcutConflict(shortcutStr)) {
        updateStatusMessage("快捷键可能与系统或其他软件冲突", true);
        okButton->setEnabled(true);
    } else {
        updateStatusMessage("快捷键有效", false);
        okButton->setEnabled(true);
    }
}

void ShortcutDialog::onResetButtonClicked()
{
    shortcutEdit->setKeySequence(QKeySequence("Ctrl+W"));
    onKeySequenceChanged(QKeySequence("Ctrl+W"));
}

void ShortcutDialog::onOkButtonClicked()
{
    QKeySequence sequence = shortcutEdit->keySequence();
    if (sequence.isEmpty()) {
        updateStatusMessage("请输入有效的快捷键组合", true);
        return;
    }

    QString shortcutStr = sequence.toString();

    if (isShortcutConflict(shortcutStr)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "快捷键冲突", 
            "您设置的快捷键可能与系统或其他常用软件冲突！\n\n"
            "是否仍然使用此快捷键？", 
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    accept();
}

void ShortcutDialog::onCancelButtonClicked()
{
    reject();
}

bool ShortcutDialog::isShortcutConflict(const QString &shortcut)
{
    QStringList conflictShortcuts = {
        "Ctrl+Alt+Del", "Ctrl+Shift+Esc", "Alt+F4", "Alt+Tab", 
        "Ctrl+Esc", "Win+E", "Win+D", "Win+L", "PrintScreen",
        "Win+R", "Win+Pause", "Ctrl+C", "Ctrl+V", "Ctrl+X", 
        "Ctrl+A", "Ctrl+Z", "Ctrl+Y", "F1", "F12", "Win+Tab"
    };

    QString normalizedShortcut = shortcut.toUpper().replace(" ", "");
    for (const QString &conflict : conflictShortcuts) {
        if (normalizedShortcut == conflict.toUpper().replace(" ", "")) {
            return true;
        }
    }

    if (shortcut.contains("Ctrl+Alt+", Qt::CaseInsensitive) || 
        shortcut.contains("Ctrl+Shift+Alt+", Qt::CaseInsensitive) ||
        shortcut.contains("Win+Ctrl", Qt::CaseInsensitive) ||
        shortcut.contains("Win+Alt", Qt::CaseInsensitive)) {
        return true;
    }

    if (shortcut.contains("Ctrl+Shift", Qt::CaseInsensitive) && 
        (shortcut.endsWith("T", Qt::CaseInsensitive) || 
         shortcut.endsWith("N", Qt::CaseInsensitive) ||
         shortcut.endsWith("W", Qt::CaseInsensitive))) {
        return true;
    }

    return false;
}

void ShortcutDialog::updateStatusMessage(const QString &message, bool isError)
{
    statusLabel->setText(message);
    if (isError) {
        statusLabel->setStyleSheet("color: #f44336; font-size: 12px;");
    } else {
        statusLabel->setStyleSheet("color: #4caf50; font-size: 12px;");
    }
}
