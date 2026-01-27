# QChatWidget 项目架构文档

## 项目概述

**QChatWidget** 是一个可移植的 Qt 对话窗口组件库,提供纯源码集成方式。

### 核心特性
- **Markdown 渲染**: 基于 md4c 的高性能 Markdown 解析
- **流式输出**: 支持逐字显示的打字机效果
- **聊天列表**: 可选的聊天会话列表组件,支持搜索过滤
- **主题管理**: 统一的样式表管理系统
- **模型配置**: 可选的 AI 模型导入界面

### 项目定位
- **纯源码库**: 不提供预编译库(静态或动态),用户通过 `.pri` 文件直接引入源码
- **模块化设计**: 各组件独立,可按需集成
- **可移植性**: 基于 Qt 5.14.2,跨平台兼容

---

## 技术栈

| 技术  | 版本   | 用途                     |
| ----- | ------ | ------------------------ |
| Qt    | 5.14.2 | UI 框架                  |
| C++   | C++17  | 编程语言                 |
| qmake | -      | 构建系统                 |
| md4c  | -      | Markdown 解析 (第三方库) |

### 第三方依赖
- **md4c**: 位于 `3rdparty/md4c/`,提供 C 语言的 Markdown 到 HTML 转换

---

## 架构设计

### 模块划分

```
QChatWidget/
├── src/
│   ├── chatwidget/      # 聊天窗口核心组件
│   ├── chatlist/        # 聊天列表组件
│   ├── common/          # 通用工具 (主题管理)
│   └── modelconfig/     # 模型配置界面
├── resources/styles/    # 样式表资源
├── test/                # 示例应用
└── 3rdparty/            # 第三方依赖
```

### 核心模块说明

#### 1. ChatWidget (聊天窗口)
**职责**: 提供完整的聊天对话界面

**核心类**:
- `ChatWidget`: 主入口组件,组合视图、输入框和模型
- `ChatWidgetView`: 消息列表视图 (基于 `QListView`)
- `ChatWidgetModel`: 消息数据模型 (基于 `QAbstractListModel`)
- `ChatWidgetDelegate`: 自定义渲染器,支持 Markdown 和流式输出
- `ChatWidgetInput`: 输入框组件
- `ChatWidgetMarkdownUtils`: Markdown 转 HTML 工具类

**依赖**:
- `md4c` (Markdown 解析)
- `common/theme_manager` (主题管理)

**集成方式**:
```qmake
include(/path/to/QChatWidget/src/chatwidget/chat_widget.pri)
```

---

#### 2. ChatList (聊天列表)
**职责**: 提供聊天会话列表,支持搜索过滤

**核心类**:
- `ChatListWidget`: 主入口组件,包含搜索栏和列表视图
- `ChatListView`: 会话列表视图
- `ChatListFilterModel`: 过滤代理模型 (基于 `QSortFilterProxyModel`)
- `ChatListDelegate`: 自定义渲染器
- `ChatListRoles`: 自定义数据角色枚举

**依赖**:
- `common/theme_manager` (主题管理)

**集成方式**:
```qmake
include(/path/to/QChatWidget/src/chatlist/chat_list.pri)
```

---

#### 3. Common (通用工具)
**职责**: 提供跨模块的通用功能

**核心类**:
- `ThemeManager`: 样式表加载和管理

**集成方式**:
自动被 `chatwidget` 和 `chatlist` 引入,无需手动 include

---

#### 4. ModelConfig (模型配置)
**职责**: 提供 AI 模型导入配置界面 (可选)

**核心类**:
- `ModelConfigImportPage`: 模型导入向导页面
- `ModelConfigProvider`: 模型厂商配置结构
- `ModelConfigField`: 模型字段定义结构

**集成方式**:
```qmake
include(/path/to/QChatWidget/src/modelconfig/modelconfig.pri)
```

---

## 构建系统

### qmake + .pri 文件组织

项目采用 **qmake** 构建系统,通过 `.pri` 文件实现模块化源码集成。

#### .pri 文件结构

每个模块提供一个 `.pri` 文件作为集成入口:

| 模块         | .pri 文件路径                     | 说明                           |
| ------------ | --------------------------------- | ------------------------------ |
| ChatWidget   | `src/chatwidget/chat_widget.pri`  | 自动引入 md4c 和 theme_manager |
| ChatList     | `src/chatlist/chat_list.pri`      | 自动引入 theme_manager         |
| ThemeManager | `src/common/theme_manager.pri`    | 通常被其他模块自动引入         |
| ModelConfig  | `src/modelconfig/modelconfig.pri` | 独立可选模块                   |

#### 典型集成示例

```qmake
# 在你的项目 .pro 文件中
QT += core gui widgets

CONFIG += c++17

# 添加源码路径
INCLUDEPATH += /path/to/QChatWidget/src

# 引入所需模块
include(/path/to/QChatWidget/src/chatwidget/chat_widget.pri)
include(/path/to/QChatWidget/src/chatlist/chat_list.pri)

# 使用组件
SOURCES += main.cpp
```

#### 资源文件管理

样式表通过 `resources/styles.qrc` 管理,各模块的 `.pri` 文件会自动引入:

```qmake
!contains(RESOURCES, $$PWD/../../resources/styles.qrc) {
    RESOURCES += $$PWD/../../resources/styles.qrc
}
```

这确保样式表资源只被添加一次,避免重复。

---

## 代码规范

详细规范请参考 [STYLE_GUIDE.md](file:///f:/B_My_Document/GitHub/QChatWidget/STYLE_GUIDE.md)

### 核心约定

#### 命名规范
- **文件名**: `lower_snake` (如 `chat_widget_view.h`)
- **类名**: `PascalCase` + 模块前缀 (如 `ChatWidget`, `ChatListView`)
- **方法**: `camelCase`
- **成员变量**: `m_` + `camelCase`
- **枚举**: 枚举名和值都带模块前缀 (如 `ChatListRoles::ChatListNameRole`)

#### 头文件组织
```cpp
#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

// 1. Qt/标准库头
#include <QWidget>

// 2. 项目内头
#include "chat_widget_view.h"

// 3. 前置声明
class ChatWidgetModel;

// 4. 类定义
class ChatWidget : public QWidget {
    Q_OBJECT
public:
    // ...
signals:
    void messageSent(const QString &text);
private:
    ChatWidgetModel *m_model;
};

#endif
```

#### 类内结构顺序
1. `public`
2. `signals`
3. `public slots`
4. `protected / protected slots`
5. `private slots`
6. `private`

#### 信号/槽规范
- 信号名使用**过去式**或 `Requested` 结尾 (如 `messageSent`, `stopRequested`)
- 使用函数指针形式连接:
  ```cpp
  connect(chat, &ChatWidget::messageSent, this, &MyClass::handleMessage);
  ```

---

## 文件组织

### 目录结构详解

```
QChatWidget/
├── .agent/                    # Agent 配置和文档
│   └── Agent.md              # 本文档
├── .git/                      # Git 版本控制
├── .gitignore                # Git 忽略规则
├── .qtcreator/               # Qt Creator 配置
├── 3rdparty/                 # 第三方依赖
│   └── md4c/                 # Markdown 解析库
├── resources/                # 资源文件
│   └── styles/               # 样式表目录
│       ├── chat_widget.qss   # 聊天窗口样式
│       ├── chat_list.qss     # 聊天列表样式
│       └── styles.qrc        # 资源文件索引
├── src/                      # 源码目录
│   ├── chatwidget/           # 聊天窗口模块
│   │   ├── chat_widget.h/cpp
│   │   ├── chat_widget_view.h/cpp
│   │   ├── chat_widget_model.h/cpp
│   │   ├── chat_widget_delegate.h/cpp
│   │   ├── chat_widget_input.h/cpp
│   │   ├── chat_widget_markdown_utils.h/cpp
│   │   └── chat_widget.pri   # 模块入口
│   ├── chatlist/             # 聊天列表模块
│   │   ├── chat_list_widget.h/cpp
│   │   ├── chat_list_view.h/cpp
│   │   ├── chat_list_delegate.h/cpp
│   │   ├── chat_list_filter_model.h/cpp
│   │   ├── chat_list_roles.h
│   │   └── chat_list.pri     # 模块入口
│   ├── common/               # 通用工具
│   │   ├── theme_manager.h/cpp
│   │   └── theme_manager.pri
│   └── modelconfig/          # 模型配置
│       ├── model_config_import_page.h/cpp
│       └── modelconfig.pri
├── test/                     # 示例应用
│   ├── we_chat_style/        # 聊天窗口示例
│   ├── we_chat_list_demo/    # 聊天列表示例
│   └── ModelImportTest/      # 模型配置示例
├── build/                    # 构建输出 (git ignored)
├── LICENSE                   # 许可证
├── README.md                 # 项目说明
└── STYLE_GUIDE.md            # 代码规范
```

### 关键文件说明

| 文件                   | 用途             |
| ---------------------- | ---------------- |
| `src/*/\*.pri`         | 模块源码集成入口 |
| `resources/styles.qrc` | 样式表资源索引   |
| `STYLE_GUIDE.md`       | 代码规范文档     |
| `README.md`            | 使用说明         |
| `.gitignore`           | 排除构建产物     |

---

## 开发工作流

### 1. 集成现有组件

**步骤**:
1. 在你的 `.pro` 文件中添加 `INCLUDEPATH`
2. 使用 `include()` 引入所需模块的 `.pri` 文件
3. 在代码中使用组件类

**示例**:
```qmake
# MyApp.pro
QT += core gui widgets
CONFIG += c++17

INCLUDEPATH += /path/to/QChatWidget/src
include(/path/to/QChatWidget/src/chatwidget/chat_widget.pri)

SOURCES += main.cpp
```

```cpp
// main.cpp
#include <QApplication>
#include "chatwidget/chat_widget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    ChatWidget *chat = new ChatWidget();
    chat->applyStyleSheetFile(":/styles/chat_widget.qss");
    chat->show();
    
    return app.exec();
}
```

---

### 2. 添加新组件

**步骤**:
1. 在 `src/` 下创建新模块目录 (如 `src/newmodule/`)
2. 按照命名规范创建 `.h` 和 `.cpp` 文件
3. 创建 `newmodule.pri` 文件,列出 `SOURCES` 和 `HEADERS`
4. 如需样式表,在 `resources/styles/` 添加 `.qss` 并更新 `styles.qrc`
5. 在 `.pri` 中引入资源文件和依赖模块

**newmodule.pri 模板**:
```qmake
NEWMODULE_DIR = $$PWD

INCLUDEPATH += $$NEWMODULE_DIR

SOURCES += \
    $$NEWMODULE_DIR/new_widget.cpp

HEADERS += \
    $$NEWMODULE_DIR/new_widget.h

!contains(RESOURCES, $$PWD/../../resources/styles.qrc) {
    RESOURCES += $$PWD/../../resources/styles.qrc
}

# 如需依赖其他模块
include($$PWD/../common/theme_manager.pri)
```

---

### 3. 构建示例应用

**独立构建目录**:
```bash
cd f:\B_My_Document\GitHub\QChatWidget
mkdir build_example && cd build_example
qmake ../test/we_chat_style/WeChatStyle.pro
mingw32-make
```

**运行**:
```bash
debug\WeChatStyle.exe
```

---

### 4. 样式定制

**方式一**: 使用内置样式
```cpp
ChatWidget *chat = new ChatWidget();
chat->applyStyleSheetFile(":/styles/chat_widget.qss");
```

**方式二**: 自定义样式
```cpp
ChatWidget *chat = new ChatWidget();
chat->setStyleSheet("QListView { background: #1e1e1e; }");
```

**方式三**: 外部样式文件
```cpp
ChatWidget *chat = new ChatWidget();
chat->applyStyleSheetFile("/path/to/custom.qss");
```

---

### 5. 最佳实践

#### 编译隔离
- **必须**在独立的 `build/` 目录中编译
- **禁止**在项目根目录直接运行 `qmake`

#### 依赖管理
- 优先使用**前置声明**,减少头文件依赖
- `.pri` 文件中只引入必要的依赖模块

#### 模块化原则
- 每个模块保持**单一职责**
- 通过 `.pri` 文件暴露最小化接口
- 避免模块间循环依赖

#### 样式表管理
- 样式表**不自动加载**,需显式调用 `applyStyleSheetFile()`
- 样式表文件统一放在 `resources/styles/`
- 使用 Qt 资源系统 (`:/styles/`) 确保可移植性

---

## 常见问题

### Q: 如何只使用 ChatWidget 而不引入 ChatList?
**A**: 只 include `chat_widget.pri`,不引入 `chat_list.pri`:
```qmake
include(/path/to/QChatWidget/src/chatwidget/chat_widget.pri)
```

### Q: 为什么不提供预编译库?
**A**: 
- 简化跨平台兼容性 (不同编译器、Qt 版本)
- 用户可自由选择编译选项 (静态/动态链接、优化级别)
- 源码集成更灵活,便于调试和定制

### Q: 如何升级到新版本?
**A**: 
1. 拉取最新代码: `git pull`
2. 重新构建你的应用 (源码会自动更新)

### Q: 样式表不生效?
**A**: 检查:
1. 是否调用了 `applyStyleSheetFile()` 或 `setStyleSheet()`
2. 资源路径是否正确 (使用 `:/styles/` 前缀)
3. `.qrc` 文件是否正确引入到 `.pri` 中

---

## 参考资料

- [README.md](file:///f:/B_My_Document/GitHub/QChatWidget/README.md) - 快速开始指南
- [STYLE_GUIDE.md](file:///f:/B_My_Document/GitHub/QChatWidget/STYLE_GUIDE.md) - 详细代码规范
- [test/we_chat_style/](file:///f:/B_My_Document/GitHub/QChatWidget/test/we_chat_style) - 完整示例应用
