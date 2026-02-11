#include "remotedesktopwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QTcpSocket>
#include <QTimer>
#include <QApplication>
#include <QStyle>
#include <QProgressDialog>
#include <QEventLoop>
#include <QStandardPaths>
#include <QTextStream>

RemoteDesktopWidget::RemoteDesktopWidget(Database *db, QWidget *parent)
    : QWidget(parent), db(db)
{
    setupUI();
    refreshConnectionList();
}

void RemoteDesktopWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *searchLabel = new QLabel("搜索:");
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("输入名称、主机地址、用户名等进行搜索...");
    connect(searchEdit, &QLineEdit::textChanged, this, &RemoteDesktopWidget::onSearchTextChanged);

    QLabel *categoryLabel = new QLabel("分类:");
    categoryFilter = new QComboBox();
    categoryFilter->addItem("全部", "");
    categoryFilter->addItem("未分类", "未分类");
    categoryFilter->addItem("工作", "工作");
    categoryFilter->addItem("个人", "个人");
    categoryFilter->addItem("测试", "测试");
    connect(categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        refreshConnectionList();
    });

    topLayout->addWidget(searchLabel);
    topLayout->addWidget(searchEdit);
    topLayout->addWidget(categoryLabel);
    topLayout->addWidget(categoryFilter);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    connectionTable = new QTableWidget();
    connectionTable->setColumnCount(7);
    connectionTable->setHorizontalHeaderLabels(QStringList() << "名称" << "主机地址" << "端口" << "用户名" << "分类" << "收藏" << "最后使用");
    connectionTable->horizontalHeader()->setStretchLastSection(true);
    connectionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    connectionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    connectionTable->setAlternatingRowColors(true);
    connectionTable->verticalHeader()->setVisible(false);
    connectionTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(connectionTable, &QTableWidget::itemSelectionChanged, this, &RemoteDesktopWidget::onConnectionSelectionChanged);
    connect(connectionTable, &QTableWidget::doubleClicked, this, [this](const QModelIndex &index) {
        Q_UNUSED(index);
        onConnect();
    });
    connect(connectionTable, &QTableWidget::customContextMenuRequested, this, &RemoteDesktopWidget::onTableContextMenuRequested);

    mainLayout->addWidget(connectionTable);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    addButton = new QPushButton("添加连接");
    addButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    connect(addButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onAddConnection);

    editButton = new QPushButton("编辑连接");
    editButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    editButton->setEnabled(false);
    connect(editButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onEditConnection);

    deleteButton = new QPushButton("删除连接");
    deleteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    deleteButton->setEnabled(false);
    connect(deleteButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onDeleteConnection);

    favoriteButton = new QPushButton("收藏");
    favoriteButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    favoriteButton->setEnabled(false);
    connect(favoriteButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onToggleFavorite);

    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(favoriteButton);
    buttonLayout->addStretch();

    connectButton = new QPushButton("连接");
    connectButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    connectButton->setEnabled(false);
    connect(connectButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onConnect);

    testButton = new QPushButton("测试连接");
    testButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    testButton->setEnabled(false);
    connect(testButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onTestConnection);

    importButton = new QPushButton("导入");
    importButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
    connect(importButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onImportConnections);

    exportButton = new QPushButton("导出");
    exportButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(exportButton, &QPushButton::clicked, this, &RemoteDesktopWidget::onExportConnections);

    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(testButton);
    buttonLayout->addWidget(importButton);
    buttonLayout->addWidget(exportButton);

    mainLayout->addLayout(buttonLayout);
}

void RemoteDesktopWidget::refreshConnectionList()
{
    QList<RemoteDesktopConnection> connections;
    QString searchText = searchEdit->text().trimmed();
    QString categoryFilterText = categoryFilter->currentData().toString();

    if (!searchText.isEmpty()) {
        connections = db->searchRemoteDesktops(searchText);
    } else {
        connections = db->getAllRemoteDesktops();
    }

    if (!categoryFilterText.isEmpty()) {
        QList<RemoteDesktopConnection> filtered;
        for (const RemoteDesktopConnection &conn : connections) {
            if (conn.category == categoryFilterText) {
                filtered.append(conn);
            }
        }
        connections = filtered;
    }

    loadConnections(connections);
}

void RemoteDesktopWidget::loadConnections(const QList<RemoteDesktopConnection> &connections)
{
    connectionTable->setRowCount(0);

    for (const RemoteDesktopConnection &conn : connections) {
        int row = connectionTable->rowCount();
        connectionTable->insertRow(row);

        connectionTable->setItem(row, 0, new QTableWidgetItem(conn.name));
        connectionTable->setItem(row, 1, new QTableWidgetItem(conn.hostAddress));
        connectionTable->setItem(row, 2, new QTableWidgetItem(QString::number(conn.port)));
        connectionTable->setItem(row, 3, new QTableWidgetItem(conn.username));
        connectionTable->setItem(row, 4, new QTableWidgetItem(conn.category));
        connectionTable->setItem(row, 5, new QTableWidgetItem(conn.isFavorite ? "★" : ""));
        connectionTable->setItem(row, 6, new QTableWidgetItem(conn.lastUsedTime.toString("yyyy-MM-dd hh:mm")));

        connectionTable->item(row, 0)->setData(Qt::UserRole, conn.id);
    }

    updateConnectionButtons();
}

void RemoteDesktopWidget::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
    refreshConnectionList();
}

void RemoteDesktopWidget::onAddConnection()
{
    RemoteDesktopDialog dialog(db, this);
    if (dialog.exec() == QDialog::Accepted) {
        RemoteDesktopConnection conn = dialog.getConnection();
        if (db->addRemoteDesktop(conn)) {
            refreshConnectionList();
            QMessageBox::information(this, "成功", "连接添加成功！");
        } else {
            QMessageBox::warning(this, "错误", "添加连接失败！");
        }
    }
}

void RemoteDesktopWidget::onEditConnection()
{
    RemoteDesktopConnection conn = getSelectedConnection();
    if (conn.id == -1) return;

    RemoteDesktopDialog dialog(db, conn, this);
    if (dialog.exec() == QDialog::Accepted) {
        RemoteDesktopConnection updatedConn = dialog.getConnection();
        updatedConn.id = conn.id;
        updatedConn.createdTime = conn.createdTime;
        if (db->updateRemoteDesktop(updatedConn)) {
            refreshConnectionList();
            QMessageBox::information(this, "成功", "连接更新成功！");
        } else {
            QMessageBox::warning(this, "错误", "更新连接失败！");
        }
    }
}

void RemoteDesktopWidget::onDeleteConnection()
{
    RemoteDesktopConnection conn = getSelectedConnection();
    if (conn.id == -1) return;

    auto reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除连接 \"%1\" 吗？").arg(conn.name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (db->deleteRemoteDesktop(conn.id)) {
            refreshConnectionList();
            QMessageBox::information(this, "成功", "连接已删除！");
        } else {
            QMessageBox::warning(this, "错误", "删除连接失败！");
        }
    }
}

void RemoteDesktopWidget::onConnect()
{
    RemoteDesktopConnection conn = getSelectedConnection();
    if (conn.id == -1) return;

    QString targetName = "TERMSRV/" + conn.hostAddress;

    if (!conn.username.isEmpty() && !conn.password.isEmpty()) {
        QString username = conn.username;
        if (!conn.domain.isEmpty()) {
            username = conn.domain + "\\" + username;
        }

        QStringList cmdkeyArgs;
        cmdkeyArgs << "/generic:" + targetName << "/user:" + username << "/pass:" + conn.password;

        QProcess::execute("cmdkey.exe", cmdkeyArgs);
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString rdpFilePath = tempDir + "/PonyWork_RDP_" + QString::number(conn.id) + ".rdp";

    QFile rdpFile(rdpFilePath);
    if (!rdpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建临时RDP文件！");
        return;
    }

    QTextStream out(&rdpFile);
    out.setCodec("UTF-8");

    QString fullAddress = conn.hostAddress;
    if (conn.port != 3389) {
        fullAddress += ":" + QString::number(conn.port);
    }

    out << "full address:s:" << fullAddress << "\n";
    out << "screen mode id:i:" << (conn.fullScreen ? "2" : "1") << "\n";
    out << "desktopwidth:i:" << conn.screenWidth << "\n";
    out << "desktopheight:i:" << conn.screenHeight << "\n";
    out << "use multimon:i:" << (conn.useAllMonitors ? "1" : "0") << "\n";
    out << "audiomode:i:" << (conn.enableAudio ? "0" : "2") << "\n";
    out << "redirectclipboard:i:" << (conn.enableClipboard ? "1" : "0") << "\n";
    out << "redirectprinters:i:" << (conn.enablePrinter ? "1" : "0") << "\n";
    out << "redirectdrives:i:" << (conn.enableDrive ? "1" : "0") << "\n";
    out << "authentication level:i:2\n";
    out << "prompt for credentials:i:0\n";
    out << "administrative session:i:0\n";

    if (!conn.username.isEmpty()) {
        QString username = conn.username;
        if (!conn.domain.isEmpty()) {
            username = conn.domain + "\\" + username;
        }
        out << "username:s:" << username << "\n";
    }

    rdpFile.close();

    conn.lastUsedTime = QDateTime::currentDateTime();
    db->updateRemoteDesktop(conn);

    QStringList args;
    args << rdpFilePath;

    QProcess::startDetached("mstsc.exe", args);
}

void RemoteDesktopWidget::onTestConnection()
{
    RemoteDesktopConnection conn = getSelectedConnection();
    if (conn.id == -1) return;

    QProgressDialog progress("正在测试连接...", "取消", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QTcpSocket *socket = new QTcpSocket(this);
    bool success = false;
    QEventLoop loop;

    connect(socket, &QTcpSocket::connected, this, [&]() {
        success = true;
        socket->disconnectFromHost();
        loop.quit();
    });

    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, [&](QAbstractSocket::SocketError err) {
        Q_UNUSED(err);
        success = false;
        loop.quit();
    });

    QTimer::singleShot(5000, &loop, &QEventLoop::quit);

    socket->connectToHost(conn.hostAddress, conn.port);
    loop.exec();

    progress.close();
    socket->deleteLater();

    if (success) {
        QMessageBox::information(this, "测试成功", "连接测试成功！主机端口可访问。");
    } else {
        QMessageBox::warning(this, "测试失败", "连接测试失败！请检查主机地址、端口和网络连接。");
    }
}

void RemoteDesktopWidget::onImportConnections()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入连接配置", "", "JSON 文件 (*.json);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件！");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        QMessageBox::warning(this, "错误", "文件格式不正确！");
        return;
    }

    QJsonArray array = doc.array();
    int imported = 0;

    for (const QJsonValue &val : array) {
        QJsonObject obj = val.toObject();
        RemoteDesktopConnection conn;
        conn.name = obj["name"].toString();
        conn.hostAddress = obj["hostAddress"].toString();
        conn.port = obj["port"].toInt(3389);
        conn.username = obj["username"].toString();
        conn.password = obj["password"].toString();
        conn.domain = obj["domain"].toString();
        conn.category = obj["category"].toString("未分类");
        conn.notes = obj["notes"].toString();
        conn.screenWidth = obj["screenWidth"].toInt(1920);
        conn.screenHeight = obj["screenHeight"].toInt(1080);
        conn.fullScreen = obj["fullScreen"].toBool(false);
        conn.useAllMonitors = obj["useAllMonitors"].toBool(false);
        conn.enableAudio = obj["enableAudio"].toBool(true);
        conn.enableClipboard = obj["enableClipboard"].toBool(true);
        conn.enablePrinter = obj["enablePrinter"].toBool(false);
        conn.enableDrive = obj["enableDrive"].toBool(false);
        conn.isFavorite = obj["isFavorite"].toBool(false);

        if (!conn.name.isEmpty() && !conn.hostAddress.isEmpty()) {
            if (db->addRemoteDesktop(conn)) {
                imported++;
            }
        }
    }

    refreshConnectionList();
    QMessageBox::information(this, "导入完成", QString("成功导入 %1 个连接！").arg(imported));
}

void RemoteDesktopWidget::onExportConnections()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出连接配置", "", "JSON 文件 (*.json);;所有文件 (*.*)");
    if (fileName.isEmpty()) return;

    QList<RemoteDesktopConnection> connections = db->getAllRemoteDesktops();
    QJsonArray array;

    for (const RemoteDesktopConnection &conn : connections) {
        QJsonObject obj;
        obj["name"] = conn.name;
        obj["hostAddress"] = conn.hostAddress;
        obj["port"] = conn.port;
        obj["username"] = conn.username;
        obj["password"] = "";
        obj["domain"] = conn.domain;
        obj["category"] = conn.category;
        obj["notes"] = conn.notes;
        obj["screenWidth"] = conn.screenWidth;
        obj["screenHeight"] = conn.screenHeight;
        obj["fullScreen"] = conn.fullScreen;
        obj["useAllMonitors"] = conn.useAllMonitors;
        obj["enableAudio"] = conn.enableAudio;
        obj["enableClipboard"] = conn.enableClipboard;
        obj["enablePrinter"] = conn.enablePrinter;
        obj["enableDrive"] = conn.enableDrive;
        obj["isFavorite"] = conn.isFavorite;
        array.append(obj);
    }

    QJsonDocument doc(array);
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        QMessageBox::information(this, "导出成功", "连接配置已成功导出！\n注意：为安全起见，密码未导出。");
    } else {
        QMessageBox::warning(this, "错误", "无法保存文件！");
    }
}

void RemoteDesktopWidget::onConnectionSelectionChanged()
{
    updateConnectionButtons();
}

void RemoteDesktopWidget::onToggleFavorite()
{
    RemoteDesktopConnection conn = getSelectedConnection();
    if (conn.id == -1) return;

    conn.isFavorite = !conn.isFavorite;
    if (db->updateRemoteDesktop(conn)) {
        refreshConnectionList();
    }
}

RemoteDesktopConnection RemoteDesktopWidget::getSelectedConnection()
{
    QList<QTableWidgetItem *> selected = connectionTable->selectedItems();
    if (selected.isEmpty()) {
        RemoteDesktopConnection empty;
        empty.id = -1;
        return empty;
    }

    int row = selected.first()->row();
    int id = connectionTable->item(row, 0)->data(Qt::UserRole).toInt();
    return db->getRemoteDesktopById(id);
}

void RemoteDesktopWidget::updateConnectionButtons()
{
    bool hasSelection = !connectionTable->selectedItems().isEmpty();
    editButton->setEnabled(hasSelection);
    deleteButton->setEnabled(hasSelection);
    connectButton->setEnabled(hasSelection);
    testButton->setEnabled(hasSelection);
    favoriteButton->setEnabled(hasSelection);

    if (hasSelection) {
        RemoteDesktopConnection conn = getSelectedConnection();
        favoriteButton->setText(conn.isFavorite ? "取消收藏" : "收藏");
    }
}

RemoteDesktopDialog::RemoteDesktopDialog(Database *db, QWidget *parent)
    : QDialog(parent), db(db), m_isEditMode(false)
{
    setupUI();
    setWindowTitle("添加远程桌面连接");
}

RemoteDesktopDialog::RemoteDesktopDialog(Database *db, const RemoteDesktopConnection &connection, QWidget *parent)
    : QDialog(parent), db(db), m_connection(connection), m_isEditMode(true)
{
    setupUI();
    setWindowTitle("编辑远程桌面连接");
    loadConnection(connection);
}

void RemoteDesktopDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QTabWidget *tabWidget = new QTabWidget();

    QWidget *generalTab = new QWidget();
    QFormLayout *generalLayout = new QFormLayout(generalTab);

    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("例如：公司服务器");
    hostEdit = new QLineEdit();
    hostEdit->setPlaceholderText("例如：192.168.1.100 或 server.example.com");
    portSpin = new QSpinBox();
    portSpin->setRange(1, 65535);
    portSpin->setValue(3389);
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("登录用户名");
    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("登录密码");
    domainEdit = new QLineEdit();
    domainEdit->setPlaceholderText("域（可选）");
    categoryCombo = new QComboBox();
    categoryCombo->addItem("未分类", "未分类");
    categoryCombo->addItem("工作", "工作");
    categoryCombo->addItem("个人", "个人");
    categoryCombo->addItem("测试", "测试");
    notesEdit = new QTextEdit();
    notesEdit->setPlaceholderText("备注信息（可选）");
    notesEdit->setMaximumHeight(80);

    generalLayout->addRow("连接名称 *:", nameEdit);
    generalLayout->addRow("主机地址 *:", hostEdit);
    generalLayout->addRow("端口:", portSpin);
    generalLayout->addRow("用户名:", usernameEdit);
    generalLayout->addRow("密码:", passwordEdit);
    generalLayout->addRow("域:", domainEdit);
    generalLayout->addRow("分类:", categoryCombo);
    generalLayout->addRow("备注:", notesEdit);

    tabWidget->addTab(generalTab, "基本信息");

    QWidget *displayTab = new QWidget();
    QFormLayout *displayLayout = new QFormLayout(displayTab);

    widthSpin = new QSpinBox();
    widthSpin->setRange(640, 7680);
    widthSpin->setValue(1920);
    heightSpin = new QSpinBox();
    heightSpin->setRange(480, 4320);
    heightSpin->setValue(1080);
    fullScreenCheck = new QCheckBox("全屏模式");
    allMonitorsCheck = new QCheckBox("使用所有显示器");

    displayLayout->addRow("屏幕宽度:", widthSpin);
    displayLayout->addRow("屏幕高度:", heightSpin);
    displayLayout->addRow("", fullScreenCheck);
    displayLayout->addRow("", allMonitorsCheck);

    tabWidget->addTab(displayTab, "显示设置");

    QWidget *resourceTab = new QWidget();
    QVBoxLayout *resourceLayout = new QVBoxLayout(resourceTab);

    audioCheck = new QCheckBox("启用音频重定向");
    audioCheck->setChecked(true);
    clipboardCheck = new QCheckBox("启用剪贴板");
    clipboardCheck->setChecked(true);
    printerCheck = new QCheckBox("启用打印机重定向");
    driveCheck = new QCheckBox("启用驱动器重定向");

    resourceLayout->addWidget(audioCheck);
    resourceLayout->addWidget(clipboardCheck);
    resourceLayout->addWidget(printerCheck);
    resourceLayout->addWidget(driveCheck);
    resourceLayout->addStretch();

    tabWidget->addTab(resourceTab, "本地资源");

    mainLayout->addWidget(tabWidget);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &RemoteDesktopDialog::onSave);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(buttonBox);

    connect(nameEdit, &QLineEdit::textChanged, this, &RemoteDesktopDialog::validateForm);
    connect(hostEdit, &QLineEdit::textChanged, this, &RemoteDesktopDialog::validateForm);

    validateForm();
}

void RemoteDesktopDialog::loadConnection(const RemoteDesktopConnection &connection)
{
    nameEdit->setText(connection.name);
    hostEdit->setText(connection.hostAddress);
    portSpin->setValue(connection.port);
    usernameEdit->setText(connection.username);
    passwordEdit->setText(connection.password);
    domainEdit->setText(connection.domain);
    widthSpin->setValue(connection.screenWidth);
    heightSpin->setValue(connection.screenHeight);
    fullScreenCheck->setChecked(connection.fullScreen);
    allMonitorsCheck->setChecked(connection.useAllMonitors);
    audioCheck->setChecked(connection.enableAudio);
    clipboardCheck->setChecked(connection.enableClipboard);
    printerCheck->setChecked(connection.enablePrinter);
    driveCheck->setChecked(connection.enableDrive);
    notesEdit->setPlainText(connection.notes);

    int categoryIndex = categoryCombo->findData(connection.category);
    if (categoryIndex >= 0) {
        categoryCombo->setCurrentIndex(categoryIndex);
    }
}

RemoteDesktopConnection RemoteDesktopDialog::getConnection() const
{
    RemoteDesktopConnection conn = m_connection;
    conn.name = nameEdit->text().trimmed();
    conn.hostAddress = hostEdit->text().trimmed();
    conn.port = portSpin->value();
    conn.username = usernameEdit->text().trimmed();
    conn.password = passwordEdit->text();
    conn.domain = domainEdit->text().trimmed();
    conn.screenWidth = widthSpin->value();
    conn.screenHeight = heightSpin->value();
    conn.fullScreen = fullScreenCheck->isChecked();
    conn.useAllMonitors = allMonitorsCheck->isChecked();
    conn.enableAudio = audioCheck->isChecked();
    conn.enableClipboard = clipboardCheck->isChecked();
    conn.enablePrinter = printerCheck->isChecked();
    conn.enableDrive = driveCheck->isChecked();
    conn.notes = notesEdit->toPlainText();
    conn.category = categoryCombo->currentData().toString();
    return conn;
}

void RemoteDesktopDialog::validateForm()
{
    bool valid = !nameEdit->text().trimmed().isEmpty() && !hostEdit->text().trimmed().isEmpty();
    buttonBox->button(QDialogButtonBox::Save)->setEnabled(valid);
}

void RemoteDesktopDialog::onSave()
{
    accept();
}

void RemoteDesktopWidget::onTableContextMenuRequested(const QPoint &pos)
{
    QMenu contextMenu(this);
    
    RemoteDesktopConnection selectedConn = getSelectedConnection();
    bool hasSelection = (selectedConn.id != -1);
    
    QAction *connectAction = contextMenu.addAction(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon), "连接");
    connectAction->setEnabled(hasSelection);
    connect(connectAction, &QAction::triggered, this, &RemoteDesktopWidget::onConnect);
    
    QAction *testAction = contextMenu.addAction(QApplication::style()->standardIcon(QStyle::SP_BrowserReload), "测试连接");
    testAction->setEnabled(hasSelection);
    connect(testAction, &QAction::triggered, this, &RemoteDesktopWidget::onTestConnection);
    
    contextMenu.addSeparator();
    
    QAction *editAction = contextMenu.addAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogDetailedView), "编辑连接");
    editAction->setEnabled(hasSelection);
    connect(editAction, &QAction::triggered, this, &RemoteDesktopWidget::onEditConnection);
    
    QAction *deleteAction = contextMenu.addAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), "删除连接");
    deleteAction->setEnabled(hasSelection);
    connect(deleteAction, &QAction::triggered, this, &RemoteDesktopWidget::onDeleteConnection);
    
    contextMenu.addSeparator();
    
    QString favoriteText = hasSelection && selectedConn.isFavorite ? "取消收藏" : "收藏";
    QIcon favoriteIcon = hasSelection && selectedConn.isFavorite 
        ? QApplication::style()->standardIcon(QStyle::SP_DialogNoButton)
        : QApplication::style()->standardIcon(QStyle::SP_DialogYesButton);
    QAction *favoriteAction = contextMenu.addAction(favoriteIcon, favoriteText);
    favoriteAction->setEnabled(hasSelection);
    connect(favoriteAction, &QAction::triggered, this, &RemoteDesktopWidget::onToggleFavorite);
    
    contextMenu.addSeparator();
    
    QAction *addAction = contextMenu.addAction(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder), "添加连接");
    connect(addAction, &QAction::triggered, this, &RemoteDesktopWidget::onAddConnection);
    
    contextMenu.exec(connectionTable->mapToGlobal(pos));
}
