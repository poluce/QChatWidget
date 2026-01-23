#ifndef CHAT_WIDGET_VIEW_H
#define CHAT_WIDGET_VIEW_H

#include <QWidget>
#include <QString>

class QListView;
class ChatWidgetModel;
class ChatWidgetDelegate;

class ChatWidgetView : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidgetView(QWidget *parent = nullptr);
    ~ChatWidgetView();

    void addMessage(const QString &content, bool isMine, const QString &sender = "User");
    void streamOutput(const QString &content);

private:
    void setupUi();

    QListView *m_chatView;
    ChatWidgetModel *m_model;
    ChatWidgetDelegate *m_delegate;
};

#endif // CHAT_WIDGET_VIEW_H
