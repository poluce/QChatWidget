#ifndef CHAT_WIDGET_VIEW_H
#define CHAT_WIDGET_VIEW_H

#include "chat_widget_delegate.h"
#include <QList>
#include <QString>
#include <QWidget>

class QListView;
class ChatWidgetModel;
class QEvent;
struct ChatWidgetMessage;

class ChatWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit ChatWidgetView(QWidget* parent = nullptr);
    ~ChatWidgetView();

    void addMessage(const QString& content, bool isMine, const QString& sender = "User",
                    const QString& senderId = QString(), const QString& avatarPath = QString());
    void setMessages(const QList<ChatWidgetMessage>& messages);
    void updateIsMine(const QString& currentUserId);
    void streamOutput(const QString& content);
    void removeLastMessage();
    void clearMessages();
    int messageCount() const;

    void setDelegateStyle(const ChatWidgetDelegate::Style& style);
    ChatWidgetDelegate::Style delegateStyle() const;

signals:
    void avatarClicked(const QString& sender, bool isMine, int row);
    void selfAvatarClicked(const QString& senderId, int row);
    void memberAvatarClicked(const QString& senderId, const QString& displayName, int row);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupUi();

    QListView* m_chatView;
    ChatWidgetModel* m_model;
    ChatWidgetDelegate* m_delegate;
};

#endif // CHAT_WIDGET_VIEW_H
