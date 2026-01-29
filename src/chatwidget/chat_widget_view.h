#ifndef CHAT_WIDGET_VIEW_H
#define CHAT_WIDGET_VIEW_H

#include "chat_widget_delegate.h"
#include <QString>
#include <QWidget>

class QListView;
class ChatWidgetModel;

class ChatWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit ChatWidgetView(QWidget* parent = nullptr);
    ~ChatWidgetView();

    void addMessage(const QString& content, bool isMine, const QString& sender = "User");
    void streamOutput(const QString& content);
    void removeLastMessage();
    void clearMessages();
    int messageCount() const;

    void setDelegateStyle(const ChatWidgetDelegate::Style& style);
    ChatWidgetDelegate::Style delegateStyle() const;

private:
    void setupUi();

    QListView* m_chatView;
    ChatWidgetModel* m_model;
    ChatWidgetDelegate* m_delegate;
};

#endif // CHAT_WIDGET_VIEW_H
