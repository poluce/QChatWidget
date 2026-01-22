#ifndef CHATVIEWWIDGET_H
#define CHATVIEWWIDGET_H

#include <QWidget>

class QListView;
class ChatModel;
class ChatDelegate;

class ChatViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatViewWidget(QWidget *parent = nullptr);
    ~ChatViewWidget();

    void addMessage(const QString &content, bool isMine, const QString &sender = "User");
    void streamOutput(const QString &content);

private:
    void setupUi();

    QListView *m_chatView;
    ChatModel *m_model;
    ChatDelegate *m_delegate;
};

#endif // CHATVIEWWIDGET_H
