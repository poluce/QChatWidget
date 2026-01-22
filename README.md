# QChatWidget

一个可移植的 Qt 对话窗口组件，支持 Markdown 渲染与流式输出。仓库内包含示例应用与可独立编译的库。

## 目录说明
- `src/chatwidget/`：组件源码（库主体）
- `test/WeChatStyle.pro`：示例应用
- `lib/QChatWidgetLib.pro`：纯库（`TEMPLATE = lib`）

## 用法一：源码直引（最简单）
在你的 `.pro` 中加入：
```
INCLUDEPATH += /path/to/QChatWidget/src
include(/path/to/QChatWidget/src/chatwidget/chatwidget.pri)
```

示例代码：
```
QChatWidget *chat = new QChatWidget(this);
layout->addWidget(chat);
connect(chat, &QChatWidget::messageSent, this, [](const QString &text){
    // 处理用户输入
});
```

## 用法二：编译成静态库
打开 `lib/QChatWidgetLib.pro` 编译即可。  
如果需要导出头文件/库，可执行 `make install`（或 `mingw32-make install`），会在构建目录输出：
- `lib/`：库文件
- `include/`：头文件

然后在你的项目中链接：
```
INCLUDEPATH += /path/to/include
LIBS += -L/path/to/lib -lQChatWidget
```

## 示例应用
`test/WeChatStyle.pro` 是完整示例，可直接运行查看效果。
