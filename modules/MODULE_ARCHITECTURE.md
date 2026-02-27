# 模块架构说明文档

## 项目结构

本项目采用模块化架构，将功能相关的代码组织到独立的模块中，以提高代码的可维护性和可读性。

```
PonyWork/
├── modules/
│   ├── core/           # 核心功能模块
│   ├── ui/             # UI资源模块（保留）
│   ├── update/         # 更新功能模块
│   ├── widgets/        # 界面组件模块
│   └── dialogs/        # 对话框模块
├── ui/                 # Qt UI文件
├── img/                # 图像资源
└── mainwindow.cpp      # 主窗口（入口）
```

## 模块说明

### 1. core 模块 (核心功能)

**路径**: `modules/core/`

**包含文件**:
- `database.h/cpp` - 数据持久化模块，管理应用数据、收藏夹、设置的存储
- `logger.h/cpp` - 日志模块，提供统一的日志记录功能
- `core.h` - 模块导出头文件

**职责**:
- 数据存储和检索
- 全局日志记录
- 应用设置管理

**依赖**: Qt Core

---

### 2. update 模块 (更新功能)

**路径**: `modules/update/`

**包含文件**:
- `updatemanager.h/cpp` - 更新管理器，处理版本检查、下载、更新逻辑
- `updatedialog.h/cpp` - 更新提示对话框
- `updateprogressdialog.h/cpp` - 更新进度对话框
- `update_module.h` - 模块导出头文件

**职责**:
- GitHub版本检查
- 更新包下载
- 自动更新安装
- 用户更新体验

**依赖**: 
- Qt Network
- Qt Core
- modules/core

---

### 3. widgets 模块 (界面组件)

**路径**: `modules/widgets/`

**包含文件**:
- `appmanagerwidget.h/cpp` - 应用管理器组件
- `fishmodewidget.h/cpp` - 鱼模式组件
- `shutdownwidget.h/cpp` - 关机管理组件
- `settingswidget.h/cpp` - 设置组件
- `collectionmanagerwidget.h/cpp` - 收藏管理器组件
- `recommendedappswidget.h/cpp` - 推荐应用组件
- `remotedesktopwidget.h/cpp` - 远程桌面组件
- `snapshotmanagerwidget.h/cpp` - 快照管理组件
- `widgets_module.h` - 模块导出头文件

**职责**:
- 各功能Tab页面的UI实现
- 用户交互逻辑处理
- 应用程序启动和管理

**依赖**:
- Qt Widgets
- Qt Gui
- modules/core
- modules/update
- modules/dialogs

---

### 4. dialogs 模块 (对话框)

**路径**: `modules/dialogs/`

**包含文件**:
- `shortcutdialog.h/cpp` - 快捷键设置对话框
- `desktopsnapshotdialog.h/cpp` - 桌面快照对话框
- `dialogs_module.h` - 模块导出头文件

**职责**:
- 模态对话框的实现
- 用户配置输入
- 系统信息展示

**依赖**:
- Qt Widgets
- modules/core

---

## 模块依赖关系

```
mainwindow.cpp
    │
    ├──► core (database, logger)
    │         ▲
    │         │
    ├──► widgets ◄──────┘
    │         │
    │         ├──► update (updatemanager)
    │         │         │
    │         └──► dialogs
    │
    └──► update (updatedialog, updateprogressdialog)
```

## 使用方式

### 引用核心模块
```cpp
#include "modules/core/database.h"
#include "modules/core/logger.h"
```

### 引用更新模块
```cpp
#include "modules/update/updatemanager.h"
#include "modules/update/updatedialog.h"
```

### 引用组件模块
```cpp
#include "modules/widgets/appmanagerwidget.h"
#include "modules/widgets/settingswidget.h"
```

### 引用对话框模块
```cpp
#include "modules/dialogs/shortcutdialog.h"
```

## 单一职责原则

每个模块都遵循单一职责原则：

| 模块 | 职责 |
|------|------|
| core | 数据持久化、日志记录 |
| update | 版本管理、更新流程 |
| widgets | UI交互、功能实现 |
| dialogs | 模态交互、用户输入 |

## 添加新模块

如需添加新模块：

1. 在 `modules/` 下创建新目录
2. 创建模块头文件用于统一导出
3. 在 `PonyWork.pro` 中添加源文件路径
4. 更新各文件的 `#include` 路径

---

**创建日期**: 2026-02-27
**项目版本**: 0.0.6+
