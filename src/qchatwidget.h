#ifndef QCHATWIDGET_H
#define QCHATWIDGET_H

#include <QWidget>
#include <QListView>
#include <QLineEdit>
#include <QPushButton>
#include "chatmodel.h"
#include "chatdelegate.h"

class QChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QChatWidget(QWidget *parent = nullptr);
    ~QChatWidget();

    // API: 添加消息
    void addMessage(const QString &content, bool isMine, const QString &sender = "User");

signals:
    // API: 消息发送信号
    void messageSent(const QString &content);

private slots:
    void onSendClicked();

private:
    void setupUi();

    QListView *m_chatView;
    ChatModel *m_model;
    ChatDelegate *m_delegate;

    QLineEdit *m_inputEdit;
    QPushButton *m_sendButton;
};

#endif // QCHATWIDGET_H
