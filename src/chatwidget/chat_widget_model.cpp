#include "chat_widget_model.h"

ChatWidgetModel::ChatWidgetModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ChatWidgetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.count();
}

QVariant ChatWidgetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.count())
        return QVariant();

    const ChatWidgetMessage &msg = m_messages[index.row()];

    switch (role) {
    case ChatWidgetSenderRole:    return msg.sender;
    case ChatWidgetContentRole:   return msg.content;
    case ChatWidgetAvatarRole:    return msg.avatarPath;
    case ChatWidgetTimestampRole: return msg.timestamp;
    case ChatWidgetIsMineRole:    return msg.isMine;
    default:            return QVariant();
    }
}

QHash<int, QByteArray> ChatWidgetModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ChatWidgetSenderRole] = "sender";
    roles[ChatWidgetContentRole] = "content";
    roles[ChatWidgetAvatarRole] = "avatar";
    roles[ChatWidgetIsMineRole] = "isMine";
    return roles;
}

void ChatWidgetModel::addMessage(const ChatWidgetMessage &message)
{
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append(message);
    endInsertRows();
}

void ChatWidgetModel::appendContentToLastMessage(const QString &content)
{
    if (m_messages.isEmpty()) return;

    m_messages.last().content.append(content);
    QModelIndex idx = index(m_messages.count() - 1, 0);
    emit dataChanged(idx, idx, {ChatWidgetContentRole});
}

void ChatWidgetModel::removeLastMessage()
{
    if (m_messages.isEmpty()) {
        return;
    }

    int lastIdx = m_messages.size() - 1;
    beginRemoveRows(QModelIndex(), lastIdx, lastIdx);
    m_messages.removeAt(lastIdx);
    endRemoveRows();
}
