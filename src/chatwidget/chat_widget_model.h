#ifndef CHAT_WIDGET_MODEL_H
#define CHAT_WIDGET_MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QHash>
#include <QList>
#include <QString>
#include <QVariant>

struct ChatWidgetMessage {
    QString sender;     // 发送者姓名
    QString content;    // 消息内容
    QString avatarPath; // 头像路径
    QDateTime timestamp;// 时间戳
    bool isMine;        // 是否是我发的消息
};

class ChatWidgetModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ChatWidgetRoles {
        ChatWidgetSenderRole = Qt::UserRole + 1,
        ChatWidgetContentRole,
        ChatWidgetAvatarRole,
        ChatWidgetTimestampRole,
        ChatWidgetIsMineRole
    };

    explicit ChatWidgetModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(const ChatWidgetMessage &message);
    void appendContentToLastMessage(const QString &content);
    void removeLastMessage();

private:
    QList<ChatWidgetMessage> m_messages;
};

#endif // CHAT_WIDGET_MODEL_H
