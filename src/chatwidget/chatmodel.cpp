#include "chatmodel.h"

ChatModel::ChatModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ChatModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.count();
}

QVariant ChatModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.count())
        return QVariant();

    const ChatMessage &msg = m_messages[index.row()];

    switch (role) {
    case SenderRole:    return msg.sender;
    case ContentRole:   return msg.content;
    case AvatarRole:    return msg.avatarPath;
    case TimestampRole: return msg.timestamp;
    case IsMineRole:    return msg.isMine;
    default:            return QVariant();
    }
}

QHash<int, QByteArray> ChatModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[SenderRole] = "sender";
    roles[ContentRole] = "content";
    roles[AvatarRole] = "avatar";
    roles[IsMineRole] = "isMine";
    return roles;
}

void ChatModel::addMessage(const ChatMessage &message)
{
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append(message);
    endInsertRows();
}

void ChatModel::appendContentToLastMessage(const QString &content)
{
    if (m_messages.isEmpty()) return;

    m_messages.last().content.append(content);
    QModelIndex idx = index(m_messages.count() - 1, 0);
    emit dataChanged(idx, idx, {ContentRole});
}