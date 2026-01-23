#ifndef Q_CHAT_WIDGET_H
#define Q_CHAT_WIDGET_H

#include <QWidget>

class QChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QChatWidget(QWidget *parent = nullptr);
    ~QChatWidget();

    // API: 添加消息
    void addMessage(const QString &content, bool isMine, const QString &sender = "User");
    
    // API: 流式输出（追加内容到最后一条消息）
    void streamOutput(const QString &content);
    void setInputWidget(class ChatWidgetInputBase *widget);
    class ChatWidgetInputBase *inputWidget() const;

signals:
    // API: 消息发送信号
    void messageSent(const QString &content);
    void stopRequested();

private slots:
    void onInputMessageSent(const QString &content);

private:
    void setupUi();

    class QVBoxLayout *m_mainLayout;
    class ChatWidgetView *m_viewWidget;
    class ChatWidgetInputBase *m_inputWidget;
};

#endif // Q_CHAT_WIDGET_H
