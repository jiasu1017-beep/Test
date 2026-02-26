# 小马办公 - PonyWork

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-v0.0.6-green.svg)](https://github.com/jiasu1017-beep/Test/releases)
[![Qt Version](https://img.shields.io/badge/Qt-5.15.2-orange.svg)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)](https://www.microsoft.com/windows)

一个功能完善的桌面办公助手应用，帮助您更高效地管理和使用电脑。

---

## 目录

- [项目概述](#项目概述)
- [主要功能](#主要功能)
- [技术栈](#技术栈)
- [项目结构](#项目结构)
- [安装与配置](#安装与配置)
- [使用方法](#使用方法)
- [贡献指南](#贡献指南)
- [许可证](#许可证)
- [联系方式](#联系方式)

---

## 项目概述

### 项目名称
**小马办公 (PonyWork)** - 您的桌面办公助手

### 项目简介
小马办公是一个基于 Qt 框架开发的桌面办公助手应用，提供应用管理、集合管理、系统托盘、定时关机等实用功能，帮助用户提高办公效率。

### 目标用户
- 需要快速启动常用应用的办公人员
- 希望批量管理和启动应用的用户
- 需要定时关机功能的用户
- 喜欢简洁高效桌面工具的用户

---

## 主要功能

### 📱 应用管理
- ✅ 添加、编辑、删除应用程序
- ✅ 支持大图标和列表两种视图模式
- ✅ 多选和框选功能
- ✅ 应用排序和收藏功能
- ✅ 批量启动和批量删除

### 📂 集合管理
- ✅ 自定义应用分组（集合）
- ✅ 批量启动集合中的所有应用
- ✅ 集合排序和管理
- ✅ 自动选择第一个集合

### 🖥️ 系统托盘
- ✅ 最小化到系统托盘
- ✅ 托盘图标显示
- ✅ 右键菜单（打开窗口/退出程序）
- ✅ 双击恢复窗口
- ✅ 可配置的关闭行为

### ⏰ 定时关机
- ✅ 定时关机功能
- ✅ 定时重启功能
- ✅ 定时休眠功能
- ✅ 取消定时任务

### 🐟 摸鱼模式
- ✅ 老板键功能
- ✅ 快速切换状态
- ✅ 伪装窗口功能

### ⚙️ 设置
- ✅ 开机自动启动
- ✅ 关闭行为设置
- ✅ 关闭提示配置
- ✅ 关于对话框

---

## 技术栈

### 编程语言
- **C++17** - 主要开发语言

### 开发框架
- **Qt 5.15.2** - 跨平台应用程序框架
  - Qt Widgets - UI 组件库
  - Qt Core - 核心功能模块
  - Qt GUI - 图形界面模块

### 编译器
- **MinGW 8.1.0** - GCC for Windows

### 构建工具
- **qmake** - Qt 构建系统
- **GNU Make** - 构建自动化工具

### 数据存储
- **JSON** - 本地数据存储格式

### 核心依赖
- Qt 5.15.2 核心库
- MinGW 运行时库

---

## 项目结构

```
PonyWork/
├── .qtcreator/              # Qt Creator 项目配置
├── img/                     # 图片资源目录
│   ├── icon.ico            # 应用程序图标 (ICO)
│   ├── icon.png            # 应用程序图标 (PNG)
│   ├── pic.png             # 关于页面插图
│   └── wechater.jpg        # 微信公众号图片
├── ui/                      # Qt Designer UI 文件
│   ├── appmanagerwidget.ui
│   ├── fishmodewidget.ui
│   ├── mainwindow.ui
│   ├── settingswidget.ui
│   └── shutdownwidget.ui
├── appmanagerwidget.cpp/h   # 应用管理模块
├── collectionmanagerwidget.cpp/h # 集合管理模块
├── database.cpp/h          # 数据存储模块
├── fishmodewidget.cpp/h    # 摸鱼模式模块
├── main.cpp                # 程序入口
├── mainwindow.cpp/h        # 主窗口模块
├── recommendedappswidget.cpp/h # 推荐应用模块
├── settingswidget.cpp/h    # 设置模块
├── shutdownwidget.cpp/h    # 定时关机模块
├── PonyWork.pro            # qmake 项目文件
├── PonyWork_resource.rc    # Windows 资源文件
├── resources.qrc           # Qt 资源文件
└── README.md               # 项目说明文档
```

### 模块说明

| 模块 | 文件名 | 功能描述 |
|------|--------|----------|
| 主窗口 | mainwindow.cpp/h | 应用程序主窗口，管理所有标签页 |
| 应用管理 | appmanagerwidget.cpp/h | 应用列表的显示、添加、删除、启动 |
| 集合管理 | collectionmanagerwidget.cpp/h | 应用集合的管理和批量启动 |
| 数据存储 | database.cpp/h | JSON 数据的读写和管理 |
| 摸鱼模式 | fishmodewidget.cpp/h | 老板键和状态切换功能 |
| 定时关机 | shutdownwidget.cpp/h | 定时关机、重启、休眠功能 |
| 设置 | settingswidget.cpp/h | 应用设置和关于对话框 |
| 推荐应用 | recommendedappswidget.cpp/h | 推荐应用的浏览和搜索 |

---

## 安装与配置

### 环境要求

#### 运行环境
- **操作系统**：Windows 7/8/10/11 (64位)
- **内存**：至少 100MB 可用内存
- **磁盘空间**：至少 50MB 可用空间

#### 开发环境
- **操作系统**：Windows 10/11
- **Qt**：5.15.2 或更高版本
- **编译器**：MinGW 8.1.0 或 MSVC 2019
- **构建工具**：qmake 和 mingw32-make

### 依赖安装

#### 1. 安装 Qt
1. 访问 [Qt 官网](https://www.qt.io/download)
2. 下载 Qt Online Installer
3. 安装时选择 Qt 5.15.2 和 MinGW 8.1.0 组件

#### 2. 配置环境变量
将以下路径添加到系统 PATH 环境变量：
```
D:\Qt\5.15.2\mingw81_64\bin
D:\Qt\Tools\mingw810_64\bin
```

### 编译项目

#### 使用 Qt Creator
1. 打开 Qt Creator
2. 文件 → 打开文件或项目
3. 选择 `PonyWork.pro`
4. 选择构建套件（MinGW 64-bit）
5. 点击左下角的「运行」按钮（绿色三角形）

#### 使用命令行
```powershell
# 1. 设置环境变量
$env:PATH = "D:\Qt\Tools\mingw810_64\bin;D:\Qt\5.15.2\mingw81_64\bin;$env:PATH"

# 2. 进入项目目录
cd f:\00AI\Test

# 3. 生成 Makefile
qmake PonyWork.pro

# 4. 编译 Release 版本
mingw32-make -f Makefile.Release

# 5. 编译 Debug 版本（可选）
mingw32-make -f Makefile.Debug
```

### 配置文件说明

#### 数据文件
- **位置**：`%APPDATA%\PonyWork\data.json`
- **格式**：JSON
- **内容**：存储应用列表、集合、设置等数据

#### 设置项
| 设置项 | 说明 | 默认值 |
|--------|------|--------|
| auto_start | 开机自动启动 | false |
| minimize_to_tray | 最小化到系统托盘 | true |
| show_close_prompt | 显示关闭提示 | true |

---

## 使用方法

### 快速开始

1. **运行程序**
   ```
   双击 release\PonyWork.exe
   ```

2. **添加应用**
   - 切换到「应用管理」标签
   - 点击「添加应用」按钮
   - 选择要添加的应用程序

3. **创建集合**
   - 切换到「集合管理」标签
   - 点击「添加集合」按钮
   - 输入集合名称并选择应用

### 核心功能演示

#### 应用管理
1. **添加应用**
   - 点击「添加应用」
   - 浏览并选择 .exe 文件
   - 填写应用信息并保存

2. **启动应用**
   - 双击应用图标启动
   - 或选择应用后点击「启动应用」

3. **多选操作**
   - Shift + 点击：连续选择
   - Ctrl + 点击：逐个选择
   - 鼠标拖拽：框选

#### 集合管理
1. **创建集合**
   - 点击「添加集合」
   - 输入集合名称
   - 选择要包含的应用

2. **批量启动**
   - 选择一个集合
   - 点击「启动集合」
   - 集合中的所有应用将依次启动

#### 系统托盘
1. **最小化到托盘**
   - 点击窗口「×」按钮
   - 程序隐藏到系统托盘

2. **恢复窗口**
   - 双击托盘图标
   - 或右键 → 「打开窗口」

3. **完全退出**
   - 右键托盘图标
   - 选择「退出程序」

#### 定时关机
1. **设置定时**
   - 切换到「定时关机」标签
   - 选择操作类型（关机/重启/休眠）
   - 设置时间
   - 点击「开始」

2. **取消定时**
   - 点击「取消」按钮

---

## 贡献指南

我们欢迎并感谢任何形式的贡献！

### 分支管理策略

```
main (主分支)
  └── 稳定的发布版本
      └── develop (开发分支)
          ├── feature/app-management (功能分支)
          ├── feature/system-tray (功能分支)
          └── bugfix/xxx (修复分支)
```

- **main**：主分支，保持稳定，只接受发布版本
- **develop**：开发分支，新功能在此集成
- **feature/*****：功能分支，从 develop 分出，完成后合并回 develop
- **bugfix/*****：修复分支，从 develop 或 main 分出

### 提交规范

#### 提交信息格式
```
<type>(<scope>): <subject>

<body>

<footer>
```

#### Type 类型
- **feat**：新功能
- **fix**：修复 bug
- **docs**：文档更新
- **style**：代码格式调整
- **refactor**：重构
- **test**：测试相关
- **chore**：构建/工具链相关

#### 示例
```
feat(系统托盘): 添加最小化到系统托盘功能

- 实现系统托盘图标显示
- 添加右键菜单
- 实现双击恢复窗口

Closes #123
```

### Pull Request 流程

1. **Fork 项目**
   - 点击 GitHub 页面右上角的 Fork 按钮

2. **克隆仓库**
   ```bash
   git clone https://github.com/你的用户名/Test.git
   cd Test
   ```

3. **创建功能分支**
   ```bash
   git checkout -b feature/你的功能名称
   ```

4. **提交更改**
   ```bash
   git add .
   git commit -m "feat(scope): 描述你的更改"
   ```

5. **推送到远程**
   ```bash
   git push origin feature/你的功能名称
   ```

6. **创建 Pull Request**
   - 回到 GitHub 项目页面
   - 点击「Compare & pull request」
   - 填写 PR 描述
   - 提交 PR

### 代码规范

- 使用 4 空格缩进
- 类名使用大驼峰命名（PascalCase）
- 函数名和变量名使用小驼峰命名（camelCase）
- 成员变量使用 m_ 前缀
- 包含必要的注释
- 遵循 Qt 编码风格

---

## 许可证

本项目采用 **MIT 许可证**。

```
MIT License

Copyright (c) 2024-2026 小马办公

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

详见 [LICENSE](LICENSE) 文件。

---

## 联系方式

### 维护者

- **jiasu** - 项目主要维护者
  - GitHub: [@jiasu1017-beep](https://github.com/jiasu1017-beep)
  - Email: 45795504+Haiezan@users.noreply.github.com

### 社区

- **微信公众号**：梁柱墙笔记
  - 获取更多办公效率技巧
  - 学习实用软件开发知识
  - 不定期分享优质资源

### 问题反馈

如果您遇到问题或有建议：

1. **GitHub Issues**：[提交 Issue](https://github.com/jiasu1017-beep/Test/issues)
2. **问题描述**：请包含以下信息
   - 操作系统版本
   - 软件版本
   - 问题的详细描述
   - 复现步骤
   - 截图（如果可能）

---

## 致谢

感谢所有为本项目做出贡献的开发者！

---

**© 2024-2026 小马办公. All rights reserved.**
