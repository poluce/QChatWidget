# Agent Guide requesting

该文档旨在帮助 AI Agent 理解和操作 `QChatWidget` 项目。

## 1. 构建命令 (Build Commands)

本项目使用 Qt 的 `qmake` 构建系统。

**构建步骤:**
1.  **创建构建目录** (推荐 Out-of-source build):
    ```bash
    mkdir build
    cd build
    ```
2.  **生成 Makefile**:
    ```bash
    qmake ../WeChatStyle.pro
    ```
3.  **编译**:
    ```bash
    # Windows (MinGW)
    mingw32-make
    # Linux/macOS
    make
    # Windows (MSVC)
    nmake
    ```

**运行**:
编译成功后，可执行文件将位于 `build` 目录（或 `debug`/`release` 子目录）中。

## 2. 代码架构 (Code Architecture)

本项目是一个基于 Qt Widgets 的桌面应用程序，主要模仿微信风格的聊天界面。

**核心模式**:
-   **Model/View 架构**: 使用 `QAbstractListModel` (`ChatModel`) 管理聊天数据，使用 `QStyledItemDelegate` (`ChatDelegate`) 负责聊天气泡的绘制。
-   **Signal/Slot**: Qt 标准的信号槽机制用于组件间通信（如发送按钮点击与消息处理）。

**主要组件**:
-   `MainWindow`: 主窗口控制器，组装 View、Model 和 Delegate。
-   `ChatModel`: 数据模型，存储消息列表。
-   `ChatDelegate`: 视图代理，自定义绘制消息气泡、头像等。

## 3. 代码规范 (Code Standards)

### 3.1 命名约定
-   **类名**: PascalCase (如 `MainWindow`, `ChatModel`)
-   **函数名**: camelCase (如 `onSendClicked`, `data`)
-   **成员变量**: `m_` 前缀 + camelCase (如 `m_chatView`, `m_model`)
-   **文件名**: 全小写 (如 `mainwindow.h`, `chatmodel.cpp`)

### 3.2 格式化
-   **缩进**: 4 个空格（推测）。
-   **头文件包含**: 分组排列，Qt 系统库优先，然后是本地头文件。

### 3.3 类型使用
-   广泛使用 Qt 数据类型 (`QString`, `QList`, `QVariant` 等)。
-   C++11 标准 (`auto`, `nullptr` 等)。

## 4. 文件组织 (File Organization)

```
/
├── WeChatStyle.pro    # qmake 项目定义文件
├── src/               # 源代码目录
│   ├── main.cpp       # 应用程序入口
│   ├── mainwindow.*   # 主窗口实现
│   ├── chatmodel.*    # 聊天数据模型
│   └── chatdelegate.* # 聊天界面绘制代理
├── build/             # (推荐) 构建目录
└── output/            # 输出目录
```

## 5. 技术栈 (Tech Stack)

-   **语言**: C++11
-   **框架**: Qt 5/6 (Core, Gui, Widgets)
-   **构建系统**: qmake
