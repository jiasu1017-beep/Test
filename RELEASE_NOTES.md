# 🐴 小马办公 v0.0.5 发布说明

## 📋 版本信息
- **版本号**: v0.0.5
- **发布日期**: 2026-02-24
- **开发者**: PonyWork

---

## ✨ 新功能

### 1. 应用程序管理
- ✅ 添加、编辑、删除快捷方式
- ✅ 启动和卸载应用程序
- ✅ 导出和导入应用列表
- ✅ 分组管理应用程序

### 2. 摸鱼模式
- ✅ 计时器功能（支持自定义时长）
- ✅ 摸鱼日历记录
- ✅ 时间统计和展示

### 3. 定时关机
- ✅ 设置倒计时关机
- ✅ 定时关机功能
- ✅ 取消关机任务

### 4. 系统设置
- ✅ 开机自启动
- ✅ 最小化到系统托盘
- ✅ 自动检查更新

### 5. 自动更新功能 ⭐
- ✅ 检查更新按钮
- ✅ GitHub Release 检测
- ✅ 更新对话框展示
- ✅ 下载和安装流程

### 6. 系统托盘
- ✅ 最小化到托盘
- ✅ 右键菜单（打开、退出）
- ✅ 托盘图标显示

---

## 🐛 问题修复

### v0.0.4 修复
- ✅ 修复桌面快照功能中文件夹应用筛选问题
- ✅ 修复程序应用无法添加到集合的问题
- ✅ 修复快速访问文件夹路径识别问题
- ✅ 优化桌面快照筛选系统性能
- ✅ 修复筛选隐藏窗口的添加问题

### v0.0.1 修复
- ✅ 修复 SIGSEGV 段错误（初始化顺序问题）
- ✅ 修复最小化到系统托盘功能
- ✅ 优化更新检查错误处理
- ✅ 修复内存泄漏问题
- ✅ 移除未使用的变量警告

---

## 📦 下载和安装

### 系统要求
- **操作系统**: Windows 7 或更高版本
- **架构**: 64位 (x64)

### 安装方法
1. 下载 `release-v0.0.5.zip`
2. 解压到任意目录
3. 双击运行 `PonyWork.exe`

### ⚠️ HTTPS/SSL 支持
如果使用"检查更新"功能，需要添加 OpenSSL 库：

**方法一：下载 OpenSSL**
1. 访问 https://slproweb.com/products/Win32OpenSSL.html
2. 下载 **Win64 OpenSSL v1.1.1 Light**
3. 安装或解压后，找到以下文件：
   - `libssl-1_1-x64.dll`
   - `libcrypto-1_1-x64.dll`
4. 将这两个文件复制到程序目录

**方法二：使用系统 OpenSSL**
- 如果系统已安装 OpenSSL，将其 bin 目录添加到 PATH

---

## 🚀 GitHub 发布方法

### 步骤 1：确保代码已提交
```bash
git status
git add .
git commit -m "v0.0.5 release"
```

### 步骤 2：创建标签
```bash
git tag v0.0.5
```

### 步骤 3：推送代码和标签到 GitHub
```bash
git push origin main
git push origin v0.0.1
```

### 步骤 4：在 GitHub 创建 Release
1. 访问 GitHub 仓库页面：https://github.com/jiasu1017-beep/Test
2. 点击右侧的 **Releases** 标签
3. 点击 **Draft a new release** 按钮
4. 填写发布信息：
   - **Choose a tag**: 选择 `v0.0.1`
   - **Release title**: `小马办公 v0.0.1`
   - **Describe this release**: 粘贴下方的发布说明内容
5. 上传发布包：
   - 点击 **Attach binaries by dropping them here or selecting them.**
   - 上传 `release-v0.0.1.zip` 文件
6. 点击 **Publish release** 完成发布

---

## 📝 GitHub Release 描述模板

```markdown
# 🐴 小马办公 v0.0.5

## ✨ 新功能
- 应用程序管理（添加、编辑、删除、分组）
- 摸鱼模式（计时器、日历、统计）
- 定时关机功能
- 最小化到系统托盘
- 自动检查更新功能
- 系统托盘右键菜单
- 桌面快照筛选系统

## 🐛 修复
- 修复桌面快照功能中文件夹应用筛选问题
- 修复程序应用无法添加到集合的问题
- 修复快速访问文件夹路径识别问题
- 优化桌面快照筛选系统性能
- 修复筛选隐藏窗口的添加问题

## 📦 下载
下载 release-v0.0.5.zip，解压后运行 PonyWork.exe

## ⚠️ 注意
如需使用检查更新功能，请添加 OpenSSL 库文件（libssl-1_1-x64.dll 和 libcrypto-1_1-x64.dll）到程序目录。
```

---

## 🔄 已知问题
- 检查更新功能需要 OpenSSL 库支持
- 暂无其他已知问题

---

## 📞 反馈和支持
如有问题或建议，欢迎在 GitHub 提交 Issue：
https://github.com/jiasu1017-beep/Test/issues

---

## 📄 许可证
本项目采用 MIT 许可证。

---

**🎉 感谢使用小马办公！**
