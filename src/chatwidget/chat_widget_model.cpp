#include "chat_widget_model.h"

ChatWidgetModel::ChatWidgetModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ChatWidgetModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_messages.count();
}

QVariant ChatWidgetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_messages.count())
        return QVariant();

    const ChatWidgetMessage& msg = m_messages[index.row()];

    switch (role) {
    case ChatWidgetSenderRole:
        return msg.sender;
    case ChatWidgetContentRole:
        return msg.content;
    case ChatWidgetAvatarRole:
        return msg.avatarPath;
    case ChatWidgetTimestampRole:
        return msg.timestamp;
    case ChatWidgetIsMineRole:
        return msg.isMine;
    case ChatWidgetSenderIdRole:
        return msg.senderId;
    case ChatWidgetMessageIdRole:
        return msg.messageId;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatWidgetModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ChatWidgetSenderRole] = "sender";
    roles[ChatWidgetContentRole] = "content";
    roles[ChatWidgetAvatarRole] = "avatar";
    roles[ChatWidgetTimestampRole] = "timestamp";
    roles[ChatWidgetIsMineRole] = "isMine";
    roles[ChatWidgetSenderIdRole] = "senderId";
    roles[ChatWidgetMessageIdRole] = "messageId";
    return roles;
}

void ChatWidgetModel::addMessage(const ChatWidgetMessage& message)
{
    beginInsertRows(QModelIndex(), m_messages.count(), m_messages.count());
    m_messages.append(message);
    endInsertRows();
}

void ChatWidgetModel::setMessages(const QList<ChatWidgetMessage>& messages)
{
    beginResetModel();
    m_messages = messages;
    endResetModel();
}

void ChatWidgetModel::appendMessages(const QList<ChatWidgetMessage>& messages)
{
    if (messages.isEmpty()) {
        return;
    }
    int start = m_messages.count();
    beginInsertRows(QModelIndex(), start, start + messages.count() - 1);
    m_messages.append(messages);
    endInsertRows();
}

void ChatWidgetModel::prependMessages(const QList<ChatWidgetMessage>& messages)
{
    if (messages.isEmpty()) {
        return;
    }
    beginInsertRows(QModelIndex(), 0, messages.count() - 1);
    QList<ChatWidgetMessage> combined = messages;
    combined.append(m_messages);
    m_messages.swap(combined);
    endInsertRows();
}

void ChatWidgetModel::updateIsMine(const QString& currentUserId)
{
    if (currentUserId.trimmed().isEmpty() || m_messages.isEmpty()) {
        return;
    }
    bool anyChanged = false;
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].senderId.isEmpty()) {
            continue;
        }
        const bool newIsMine = m_messages[i].senderId == currentUserId;
        if (m_messages[i].isMine != newIsMine) {
            m_messages[i].isMine = newIsMine;
            anyChanged = true;
        }
    }
    if (anyChanged) {
        QModelIndex first = index(0, 0);
        QModelIndex last = index(m_messages.size() - 1, 0);
        emit dataChanged(first, last, { ChatWidgetIsMineRole });
    }
}

void ChatWidgetModel::updateParticipantInfo(const QString& senderId, const QString& displayName, const QString& avatarPath)
{
    if (senderId.trimmed().isEmpty() || m_messages.isEmpty()) {
        return;
    }
    bool anyChanged = false;
    for (int i = 0; i < m_messages.size(); ++i) {
        if (m_messages[i].senderId != senderId) {
            continue;
        }
        if (!displayName.isEmpty() && m_messages[i].sender != displayName) {
            m_messages[i].sender = displayName;
            anyChanged = true;
        }
        if (!avatarPath.isEmpty() && m_messages[i].avatarPath != avatarPath) {
            m_messages[i].avatarPath = avatarPath;
            anyChanged = true;
        }
    }
    if (anyChanged) {
        QModelIndex first = index(0, 0);
        QModelIndex last = index(m_messages.size() - 1, 0);
        emit dataChanged(first, last, { ChatWidgetSenderRole, ChatWidgetAvatarRole });
    }
}

void ChatWidgetModel::appendContentToLastMessage(const QString& content)
{
    if (m_messages.isEmpty())
        return;

    m_messages.last().content.append(content);
    QModelIndex idx = index(m_messages.count() - 1, 0);
    emit dataChanged(idx, idx, { ChatWidgetContentRole });
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

void ChatWidgetModel::clearMessages()
{
    if (m_messages.isEmpty())
        return;
    beginRemoveRows(QModelIndex(), 0, m_messages.size() - 1);
    m_messages.clear();
    endRemoveRows();
}

int ChatWidgetModel::messageCount() const
{
    return m_messages.size();
}
