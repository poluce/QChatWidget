#ifndef CHATMODEL_H
#define CHATMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

struct ChatMessage {
    QString sender;     // 发送者姓名
    QString content;    // 消息内容
    QString avatarPath; // 头像路径
    QDateTime timestamp;// 时间戳
    bool isMine;        // 是否是我发的消息
};

class ChatModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ChatRoles {
        SenderRole = Qt::UserRole + 1,
        ContentRole,
        AvatarRole,
        TimestampRole,
        IsMineRole
    };

    explicit ChatModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(const ChatMessage &message);

private:
    QList<ChatMessage> m_messages;
};

#endif // CHATMODEL_H