#ifndef CHAT_WIDGET_VIEW_H
#define CHAT_WIDGET_VIEW_H

#include "chat_widget_delegate.h"
#include "chat_widget_model.h"
#include <QList>
#include <QPoint>
#include <QString>
#include <QWidget>

class QListView;
class QEvent;

class ChatWidgetView : public QWidget {
    Q_OBJECT

public:
    explicit ChatWidgetView(QWidget* parent = nullptr);
    ~ChatWidgetView();

    void addMessage(const QString& content, bool isMine, const QString& sender = "User",
                    const QString& senderId = QString(), const QString& avatarPath = QString());
    void setMessages(const QList<ChatWidgetMessage>& messages);
    void appendMessages(const QList<ChatWidgetMessage>& messages);
    void prependMessages(const QList<ChatWidgetMessage>& messages);
    void updateIsMine(const QString& currentUserId);
    void updateParticipantInfo(const QString& senderId, const QString& displayName, const QString& avatarPath);
    void streamOutput(const QString& content);
    void updateMessageStatus(const QString& messageId, ChatWidgetMessage::MessageStatus status);
    void updateMessageContent(const QString& messageId, const QString& content);
    void updateMessageReactions(const QString& messageId, const QList<ChatWidgetReaction>& reactions);
    void updateMessageAttachments(const QString& messageId, const QString& imagePath, const QString& filePath,
                                  const QString& fileName, qint64 fileSize);
    void updateMessageReply(const QString& messageId, const QString& replyToMessageId, const QString& replySender,
                            const QString& replyPreview, bool isForwarded, const QString& forwardedFrom);
    void setSearchKeyword(const QString& keyword);
    void removeLastMessage();
    void clearMessages();
    int messageCount() const;

    void setDelegateStyle(const ChatWidgetDelegate::Style& style);
    ChatWidgetDelegate::Style delegateStyle() const;

signals:
    void avatarClicked(const QString& sender, bool isMine, int row);
    void selfAvatarClicked(const QString& senderId, int row);
    void memberAvatarClicked(const QString& senderId, const QString& displayName, int row);
    void messageSelected(const QString& messageId);
    void messageContextMenuRequested(const QString& messageId, const QPoint& globalPos);
    void messageActionRequested(const QString& action, const QString& messageId);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupUi();

    QListView* m_chatView;
    ChatWidgetModel* m_model;
    ChatWidgetDelegate* m_delegate;
};

#endif // CHAT_WIDGET_VIEW_H
