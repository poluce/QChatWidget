#ifndef QCHATWIDGET_H
#define QCHATWIDGET_H

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
    void setInputWidget(class ChatInputWidgetBase *widget);
    class ChatInputWidgetBase *inputWidget() const;

signals:
    // API: 消息发送信号
    void messageSent(const QString &content);
    void stopRequested();

private slots:
    void onInputMessageSent(const QString &content);

private:
    void setupUi();

    class QVBoxLayout *m_mainLayout;
    class ChatViewWidget *m_viewWidget;
    class ChatInputWidgetBase *m_inputWidget;
};

#endif // QCHATWIDGET_H
