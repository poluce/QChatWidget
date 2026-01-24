#include <QApplication>
#include <QTimer>
#include <QDebug>
#include "chatwidget/chat_widget.h"
#include "common/theme_manager.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    // 1. 创建 ChatWidget
    ChatWidget chat;
    chat.setWindowTitle("ChatWidget 测试 - main.cpp 版");
    chat.resize(500, 700);
    chat.applyStyleSheetFile("chat_widget.qss");
    chat.show();

    // 2. 初始化 welcome 消息
    chat.addMessage("**Hello!** I am the component test in `main.cpp`.\nAI streaming is now internally managed.", false, "AI");

    // 3. 业务逻辑：连接发送信号并调用组件内置的模拟流式输出
    QObject::connect(&chat, &ChatWidget::messageSent, [&](const QString &content)
                     {
        // 延迟一秒模拟思考，然后直接调用组件内部的流式输出接口
        QTimer::singleShot(1000, [&chat, content]() {
            chat.startSimulatedStreaming(content); 
        }); });

    // 4. 处理停止信号
    QObject::connect(&chat, &ChatWidget::stopRequested, []()
                     { qDebug() << "User requested stop. Timer handled internally by setSendingState(false)."; });

    // 5. 应用默认主题样式
    auto applyTheme = [&]()
    {
        auto themeStyle = ThemeManager::instance()->chatWidgetStyle();
        ChatWidgetDelegate::Style style;
        style.avatarSize = themeStyle.avatarSize;
        style.margin = themeStyle.margin;
        style.bubblePadding = themeStyle.bubblePadding;
        style.bubbleRadius = themeStyle.bubbleRadius;
        style.myBubbleColor = themeStyle.myBubbleColor;
        style.otherBubbleColor = themeStyle.otherBubbleColor;
        style.myAvatarColor = themeStyle.myAvatarColor;
        style.otherAvatarColor = themeStyle.otherAvatarColor;
        style.myTextColor = themeStyle.myTextColor;
        style.otherTextColor = themeStyle.otherTextColor;
        style.messageFont = themeStyle.messageFont;
        style.avatarFont = themeStyle.avatarFont;
        chat.setDelegateStyle(style);
    };
    applyTheme();

    return app.exec();
}