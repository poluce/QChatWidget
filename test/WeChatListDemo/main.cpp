#include <QApplication>
#include <QVBoxLayout>
#include "chat_list_widget.h"
#include "chat_list_roles.h"

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Qt WeChat List Demo (Modular)");
    window.resize(360, 600);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    ChatListWidget *chatWidget = new ChatListWidget();
    chatWidget->enableSearchFiltering(true);
    chatWidget->setSearchRoles(QList<int>() << NameRole << MessageRole);

    auto addChat = [&](const QString &name, const QString &msg, const QString &time, QColor color, int unread) {
        chatWidget->addChatItem(name, msg, time, color, unread);
    };

    addChat("文件传输助手", "[图片] IMG_2026.jpg", "17:52", QColor(255, 170, 0), 0);
    addChat("腾讯新闻", "Qt 6.8 发布了！", "14:30", QColor(0, 120, 215), 1);
    addChat("产品经理", "今晚加班吗？", "12:05", QColor(100, 100, 100), 5);
    
    for(int i = 0; i < 15; ++i) {
         addChat(QString("群聊 %1").arg(i), "收到请回复", "10:00", QColor(Qt::lightGray), 0);
    }

    layout->addWidget(chatWidget);

    window.show();
    return app.exec();
}
