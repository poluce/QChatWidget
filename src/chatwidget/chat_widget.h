#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#include "chat_widget_delegate.h"
#include "chat_widget_model.h"
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QSet>
#include <QString>
#include <QWidget>

class ChatWidgetView;
class ChatWidgetInputBase;
class QTimer;

class ChatWidget : public QWidget {
    Q_OBJECT

public:
    struct ParticipantInfo {
        QString id;
        QString displayName;
        QString avatarPath;
    };
    struct HistoryMessage {
        QString senderId;
        QString displayName;
        QString avatarPath;
        QString content;
        QDateTime timestamp;
        bool isMine = false;
        QString messageId;

        ChatWidgetMessage::MessageType messageType = ChatWidgetMessage::MessageType::Text;
        ChatWidgetMessage::MessageStatus status = ChatWidgetMessage::MessageStatus::Sent;

        QString imagePath;
        QString filePath;
        QString fileName;
        qint64 fileSize = 0;

        QString replyToMessageId;
        QString replySender;
        QString replyPreview;
        bool isForwarded = false;
        QString forwardedFrom;

        QList<ChatWidgetReaction> reactions;
        QStringList mentions;
    };

    explicit ChatWidget(QWidget* parent = nullptr);
    ~ChatWidget();

    // API: 添加消息
    void addMessage(const QString& content, bool isMine, const QString& sender = "User");
    void addMessage(const QString& content, const QString& senderId);
    void addMessage(const QString& content, const QString& senderId, const QString& displayName, const QString& avatarPath = QString());

    // API: 流式输出（追加内容到最后一条消息）
    void streamOutput(const QString& content);

    // API: 移除最后一条消息
    void removeLastMessage();
    void clearMessages();
    int messageCount() const;
    bool applyStyleSheetFile(const QString& fileNameOrPath);
    void setDelegateStyle(const ChatWidgetDelegate::Style& style);
    ChatWidgetDelegate::Style delegateStyle() const;
    void setInputWidget(class ChatWidgetInputBase* widget);
    class ChatWidgetInputBase* inputWidget() const;
    void setSendingState(bool sending);
    void setEmptyStateVisible(bool visible, const QString& message = QString());
    bool isEmptyStateVisible() const;
    void setCurrentUserId(const QString& userId);
    QString currentUserId() const;
    void setCurrentUser(const QString& userId, const QString& displayName = QString(), const QString& avatarPath = QString());
    void upsertParticipant(const QString& userId, const QString& displayName, const QString& avatarPath = QString());
    void updateParticipant(const QString& userId, const QString& displayName, const QString& avatarPath = QString());
    bool removeParticipant(const QString& userId);
    void clearParticipants();
    bool hasParticipant(const QString& userId) const;
    void setHistoryMessages(const QList<HistoryMessage>& messages, bool resetParticipants = true);
    void appendHistoryMessages(const QList<HistoryMessage>& messages, bool sortAndDedupe = true);
    void prependHistoryMessages(const QList<HistoryMessage>& messages, bool sortAndDedupe = true);

    void updateMessageStatus(const QString& messageId, ChatWidgetMessage::MessageStatus status);
    void updateMessageContent(const QString& messageId, const QString& content);
    void updateMessageReactions(const QString& messageId, const QList<ChatWidgetReaction>& reactions);
    void updateMessageAttachments(const QString& messageId, const QString& imagePath, const QString& filePath,
                                  const QString& fileName, qint64 fileSize);
    void updateMessageReply(const QString& messageId, const QString& replyToMessageId, const QString& replySender,
                            const QString& replyPreview, bool isForwarded, const QString& forwardedFrom);
    void setSearchKeyword(const QString& keyword);

    // API: 模拟 AI 自动流式回复（组件内部管理定时器）
    void startSimulatedStreaming(const QString& content, int interval = 30);

signals:
    // API: 消息发送信号
    void messageSent(const QString& content);
    void avatarClicked(const QString& sender, bool isMine, int row);
    void selfAvatarClicked(const QString& senderId, int row);
    void memberAvatarClicked(const QString& senderId, const QString& displayName, int row);
    void stopRequested();
    void messageSelected(const QString& messageId);
    void messageContextMenuRequested(const QString& messageId, const QPoint& globalPos);
    void messageActionRequested(const QString& action, const QString& messageId);

private slots:
    void onInputMessageSent(const QString& content);
    void onStreamingTimeout();

private:
    void setupUi();

    class QVBoxLayout* m_mainLayout;
    class ChatWidgetView* m_viewWidget;
    class ChatWidgetInputBase* m_inputWidget;
    bool m_isSending = false;
    QHash<QString, ParticipantInfo> m_participants;
    QString m_currentUserId;
    QSet<QString> m_messageIds;

    QTimer* m_streamingTimer = nullptr;
    QString m_streamingContent;
    int m_streamingIndex = 0;
};

#endif // CHAT_WIDGET_H
