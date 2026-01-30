#ifndef CHAT_WIDGET_MODEL_H
#define CHAT_WIDGET_MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>

struct ChatWidgetMessage {
    QString senderId;    // 发送者ID（多人聊天用）
    QString sender;      // 发送者姓名
    QString content;     // 消息内容
    QString avatarPath;  // 头像路径
    QDateTime timestamp; // 时间戳
    bool isMine;         // 是否是我发的消息
    QString messageId;   // 消息唯一ID
};

class ChatWidgetModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum ChatWidgetRoles {
        ChatWidgetSenderRole = Qt::UserRole + 1,
        ChatWidgetContentRole,
        ChatWidgetAvatarRole,
        ChatWidgetTimestampRole,
        ChatWidgetIsMineRole,
        ChatWidgetSenderIdRole,
        ChatWidgetMessageIdRole
    };

    explicit ChatWidgetModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(const ChatWidgetMessage& message);
    void setMessages(const QList<ChatWidgetMessage>& messages);
    void appendMessages(const QList<ChatWidgetMessage>& messages);
    void prependMessages(const QList<ChatWidgetMessage>& messages);
    void updateIsMine(const QString& currentUserId);
    void updateParticipantInfo(const QString& senderId, const QString& displayName, const QString& avatarPath);
    void appendContentToLastMessage(const QString& content);
    void removeLastMessage();
    void clearMessages();
    int messageCount() const;

private:
    QList<ChatWidgetMessage> m_messages;
};

#endif // CHAT_WIDGET_MODEL_H
