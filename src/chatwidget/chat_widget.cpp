#include "chat_widget.h"
#include "chat_widget_input.h"
#include "chat_widget_model.h"
#include "chat_widget_view.h"
#include "qss_utils.h"
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

ChatWidget::ChatWidget(QWidget* parent) : QWidget(parent)
{
    m_streamingTimer = new QTimer(this);
    connect(m_streamingTimer, &QTimer::timeout, this, &ChatWidget::onStreamingTimeout);
    setupUi();
}

ChatWidget::~ChatWidget() { }

ChatWidgetView* ChatWidget::view() const
{
    return m_viewWidget;
}

ChatWidgetModel* ChatWidget::model() const
{
    return m_viewWidget ? m_viewWidget->model() : nullptr;
}

void ChatWidget::setupUi()
{
    m_viewWidget = new ChatWidgetView(this);
    m_inputWidget = new ChatWidgetInput(this);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_viewWidget);
    m_mainLayout->addWidget(m_inputWidget);

    connect(m_viewWidget, &ChatWidgetView::avatarClicked, this, &ChatWidget::avatarClicked);
    connect(m_viewWidget, &ChatWidgetView::selfAvatarClicked, this, &ChatWidget::selfAvatarClicked);
    connect(m_viewWidget, &ChatWidgetView::memberAvatarClicked, this, &ChatWidget::memberAvatarClicked);
    connect(m_viewWidget, &ChatWidgetView::messageSelected, this, &ChatWidget::messageSelected);
    connect(m_viewWidget, &ChatWidgetView::messageContextMenuRequested, this, &ChatWidget::messageContextMenuRequested);
    connect(m_viewWidget, &ChatWidgetView::messageActionRequested, this, &ChatWidget::messageActionRequested);
    connect(m_inputWidget, &ChatWidgetInputBase::messageSent, this, &ChatWidget::onInputMessageSent);
    connect(m_inputWidget, &ChatWidgetInputBase::stopRequested, this, [this]() {
    setSendingState(false);
    emit stopRequested(); });
}

void ChatWidget::addMessage(const MessageParams& params)
{
    if (params.senderId.trimmed().isEmpty()) {
        const QString fallbackName = params.displayName.isEmpty() ? QStringLiteral("User") : params.displayName;
        ChatWidgetMessage msg;
        msg.senderId = QString();
        msg.sender = fallbackName;
        msg.content = params.content;
        msg.avatarPath = params.avatarPath;
        msg.isMine = params.isMine;
        msg.timestamp = QDateTime::currentDateTime();

        if (auto* dataModel = model()) {
            dataModel->addMessage(msg);
        }
        if (m_viewWidget) {
            m_viewWidget->scrollToBottom();
        }
        return;
    }

    ParticipantInfo info = m_participants.value(params.senderId);
    info.id = params.senderId;
    if (!params.displayName.isEmpty()) {
        info.displayName = params.displayName;
    }
    if (!params.avatarPath.isEmpty()) {
        info.avatarPath = params.avatarPath;
    }
    m_participants.insert(params.senderId, info);

    const QString finalName = info.displayName.isEmpty() ? params.senderId : info.displayName;
    bool isMine = params.isMine;
    if (!m_currentUserId.isEmpty()) {
        isMine = params.senderId == m_currentUserId;
    }
    ChatWidgetMessage msg;
    msg.senderId = params.senderId;
    msg.sender = finalName;
    msg.content = params.content;
    msg.avatarPath = info.avatarPath;
    msg.isMine = isMine;
    msg.timestamp = QDateTime::currentDateTime();

    if (auto* dataModel = model()) {
        dataModel->addMessage(msg);
    }
    if (m_viewWidget) {
        m_viewWidget->scrollToBottom();
    }
}

void ChatWidget::streamOutput(const QString& content)
{
    // 物理拦截：如果当前不处于发送/接收状态，则拒绝任何字符追加
    if (!m_isSending) {
        return;
    }
    if (auto* dataModel = model()) {
        dataModel->appendContentToLastMessage(content);
    }
    if (m_viewWidget) {
        m_viewWidget->scrollToBottom();
    }
}

bool ChatWidget::applyStyleSheetFile(const QString& fileNameOrPath)
{
    if (fileNameOrPath.trimmed().isEmpty()) {
        return false;
    }
    return QssUtils::applyStyleSheetFromFile(this, fileNameOrPath);
}

void ChatWidget::setDelegateStyle(const ChatWidgetDelegate::Style& style)
{
    m_viewWidget->setDelegateStyle(style);
}

ChatWidgetDelegate::Style ChatWidget::delegateStyle() const
{
    return m_viewWidget->delegateStyle();
}

void ChatWidget::setInputWidget(ChatWidgetInputBase* widget)
{
    if (!widget || widget == m_inputWidget) {
        return;
    }

    if (m_inputWidget) {
        m_mainLayout->removeWidget(m_inputWidget);
        m_inputWidget->deleteLater();
    }

    widget->setParent(this);
    m_inputWidget = widget;
    m_mainLayout->addWidget(m_inputWidget);
    connect(m_inputWidget, &ChatWidgetInputBase::messageSent, this, &ChatWidget::onInputMessageSent);
    connect(m_inputWidget, &ChatWidgetInputBase::stopRequested, this, &ChatWidget::stopRequested);
}

ChatWidgetInputBase* ChatWidget::inputWidget() const { return m_inputWidget; }

void ChatWidget::onInputMessageSent(const QString& content)
{
    m_isSending = true; // 用户点击发送后，组件自动进入“发送/等待响应”状态
    MessageParams params;
    params.content = content;
    if (!m_currentUserId.isEmpty()) {
        params.senderId = m_currentUserId;
    } else {
        params.isMine = true;
        params.displayName = QStringLiteral("Me");
    }
    addMessage(params);
    emit messageSent(content);
}

void ChatWidget::removeLastMessage()
{
    if (auto* dataModel = model()) {
        dataModel->removeLastMessage();
    }
}

void ChatWidget::clearMessages()
{
    if (auto* dataModel = model()) {
        dataModel->clearMessages();
    }
    m_messageIds.clear();
}

int ChatWidget::messageCount() const
{
    return model() ? model()->messageCount() : 0;
}

void ChatWidget::setSendingState(bool sending)
{
    m_isSending = sending; // 更新组件全局状态锁
    if (auto* input = qobject_cast<ChatWidgetInput*>(m_inputWidget)) {
        input->setSendingState(sending);
    }

    // 核心逻辑：当停止发送时，物理停止内置模拟定时器
    if (!sending && m_streamingTimer->isActive()) {
        m_streamingTimer->stop();
    }
}

void ChatWidget::setEmptyStateVisible(bool visible, const QString& message)
{
    Q_UNUSED(message);
    if (m_viewWidget) {
        m_viewWidget->setVisible(!visible);
    }
    if (m_inputWidget) {
        m_inputWidget->setVisible(!visible);
        m_inputWidget->setEnabled(!visible);
    }
    if (visible) {
        setSendingState(false);
    }
}

bool ChatWidget::isEmptyStateVisible() const
{
    return m_viewWidget && !m_viewWidget->isVisible();
}

void ChatWidget::startSimulatedStreaming(const QString& content, int interval)
{
    m_streamingContent = content;
    m_streamingIndex = 0;
    // 先添加一个空消息作为容器
    MessageParams params;
    params.content = QString();
    params.displayName = QStringLiteral("AI");
    params.isMine = false;
    addMessage(params);
    setSendingState(true);
    m_streamingTimer->start(interval);
}

void ChatWidget::updateMessageStatus(const QString& messageId, ChatWidgetMessage::MessageStatus status)
{
    if (auto* dataModel = model()) {
        dataModel->updateMessageStatus(messageId, status);
    }
}

void ChatWidget::updateMessageContent(const QString& messageId, const QString& content)
{
    if (auto* dataModel = model()) {
        dataModel->updateMessageContent(messageId, content);
    }
    if (m_viewWidget) {
        m_viewWidget->refreshLayout();
    }
}

void ChatWidget::updateMessageReactions(const QString& messageId, const QList<ChatWidgetReaction>& reactions)
{
    if (auto* dataModel = model()) {
        dataModel->updateMessageReactions(messageId, reactions);
    }
    if (m_viewWidget) {
        m_viewWidget->refreshLayout();
    }
}

void ChatWidget::updateMessageAttachments(const QString& messageId, const QString& imagePath, const QString& filePath,
                                          const QString& fileName, qint64 fileSize)
{
    if (auto* dataModel = model()) {
        dataModel->updateMessageAttachments(messageId, imagePath, filePath, fileName, fileSize);
    }
    if (m_viewWidget) {
        m_viewWidget->refreshLayout();
    }
}

void ChatWidget::updateMessageReply(const QString& messageId, const QString& replyToMessageId, const QString& replySender,
                                    const QString& replyPreview, bool isForwarded, const QString& forwardedFrom)
{
    if (auto* dataModel = model()) {
        dataModel->updateMessageReply(messageId, replyToMessageId, replySender, replyPreview, isForwarded, forwardedFrom);
    }
    if (m_viewWidget) {
        m_viewWidget->refreshLayout();
    }
}

void ChatWidget::setSearchKeyword(const QString& keyword)
{
    if (auto* dataModel = model()) {
        dataModel->setSearchKeyword(keyword);
    }
}

QString ChatWidget::currentUserId() const
{
    return m_currentUserId;
}

void ChatWidget::setCurrentUser(const QString& userId, const QString& displayName, const QString& avatarPath)
{
    if (m_currentUserId == userId && displayName.isEmpty() && avatarPath.isEmpty()) {
        return;
    }
    if (!userId.trimmed().isEmpty()) {
        upsertParticipant(userId, displayName, avatarPath);
    }
    m_currentUserId = userId;
    if (auto* dataModel = model()) {
        dataModel->updateIsMine(m_currentUserId);
    }
    if (m_viewWidget) {
        m_viewWidget->refreshLayout();
    }
}

void ChatWidget::upsertParticipant(const QString& userId, const QString& displayName, const QString& avatarPath)
{
    ParticipantInfo info;
    info.id = userId;
    info.displayName = displayName;
    info.avatarPath = avatarPath;
    m_participants.insert(userId, info);
    if ((!displayName.isEmpty() || !avatarPath.isEmpty())) {
        const QString finalName = displayName.isEmpty() ? userId : displayName;
        if (auto* dataModel = model()) {
            dataModel->updateParticipantInfo(userId, finalName, avatarPath);
        }
        if (m_viewWidget) {
            m_viewWidget->refreshLayout();
        }
    }
}

bool ChatWidget::removeParticipant(const QString& userId)
{
    return m_participants.remove(userId) > 0;
}

void ChatWidget::clearParticipants()
{
    m_participants.clear();
}

bool ChatWidget::hasParticipant(const QString& userId) const
{
    return m_participants.contains(userId);
}

void ChatWidget::setHistoryMessages(const QList<HistoryMessage>& messages, bool resetParticipants)
{
    ParticipantInfo currentInfo;
    const bool hasCurrent = !m_currentUserId.isEmpty() && m_participants.contains(m_currentUserId);
    if (hasCurrent) {
        currentInfo = m_participants.value(m_currentUserId);
    }

    if (resetParticipants) {
        m_participants.clear();
        if (hasCurrent) {
            m_participants.insert(m_currentUserId, currentInfo);
        }
        m_messageIds.clear();
    }

    QList<HistoryMessage> sorted = messages;
    std::sort(sorted.begin(), sorted.end(), [](const HistoryMessage& a, const HistoryMessage& b) {
        const qint64 at = a.timestamp.isValid() ? a.timestamp.toMSecsSinceEpoch() : 0;
        const qint64 bt = b.timestamp.isValid() ? b.timestamp.toMSecsSinceEpoch() : 0;
        return at < bt;
    });

    QList<ChatWidgetMessage> converted;
    converted.reserve(sorted.size());
    for (const HistoryMessage& history : sorted) {
        if (!history.messageId.isEmpty() && m_messageIds.contains(history.messageId)) {
            continue;
        }

        ChatWidgetMessage msg;
        msg.messageId = history.messageId;
        msg.content = history.content;
        msg.timestamp = history.timestamp;
        msg.messageType = history.messageType;
        msg.status = history.status;
        msg.imagePath = history.imagePath;
        msg.filePath = history.filePath;
        msg.fileName = history.fileName;
        msg.fileSize = history.fileSize;
        msg.replyToMessageId = history.replyToMessageId;
        msg.replySender = history.replySender;
        msg.replyPreview = history.replyPreview;
        msg.isForwarded = history.isForwarded;
        msg.forwardedFrom = history.forwardedFrom;
        msg.reactions = history.reactions;
        msg.mentions = history.mentions;

        if (history.senderId.trimmed().isEmpty()) {
            msg.senderId = QString();
            msg.sender = history.displayName.isEmpty() ? QStringLiteral("User") : history.displayName;
            msg.avatarPath = history.avatarPath;
            msg.isMine = history.isMine;
        } else {
            ParticipantInfo info = m_participants.value(history.senderId);
            info.id = history.senderId;
            if (!history.displayName.isEmpty()) {
                info.displayName = history.displayName;
            }
            if (!history.avatarPath.isEmpty()) {
                info.avatarPath = history.avatarPath;
            }
            m_participants.insert(history.senderId, info);

            msg.senderId = history.senderId;
            msg.sender = info.displayName.isEmpty() ? history.senderId : info.displayName;
            msg.avatarPath = info.avatarPath;
            msg.isMine = !m_currentUserId.isEmpty() && history.senderId == m_currentUserId;
            if (m_currentUserId.isEmpty()) {
                msg.isMine = history.isMine;
            }
        }

        if (!msg.messageId.isEmpty()) {
            m_messageIds.insert(msg.messageId);
        }
        converted.append(msg);
    }

    if (m_viewWidget) {
        m_viewWidget->setMessages(converted);
    }
}

void ChatWidget::appendHistoryMessages(const QList<HistoryMessage>& messages, bool sortAndDedupe)
{
    if (messages.isEmpty()) {
        return;
    }
    QList<HistoryMessage> sorted = messages;
    if (sortAndDedupe) {
        std::sort(sorted.begin(), sorted.end(), [](const HistoryMessage& a, const HistoryMessage& b) {
            const qint64 at = a.timestamp.isValid() ? a.timestamp.toMSecsSinceEpoch() : 0;
            const qint64 bt = b.timestamp.isValid() ? b.timestamp.toMSecsSinceEpoch() : 0;
            return at < bt;
        });
    }
    QList<ChatWidgetMessage> converted;
    converted.reserve(sorted.size());
    QHash<QString, ParticipantInfo> updatedParticipants;
    for (const HistoryMessage& history : sorted) {
        if (sortAndDedupe && !history.messageId.isEmpty() && m_messageIds.contains(history.messageId)) {
            continue;
        }

        ChatWidgetMessage msg;
        msg.messageId = history.messageId;
        msg.content = history.content;
        msg.timestamp = history.timestamp;
        msg.messageType = history.messageType;
        msg.status = history.status;
        msg.imagePath = history.imagePath;
        msg.filePath = history.filePath;
        msg.fileName = history.fileName;
        msg.fileSize = history.fileSize;
        msg.replyToMessageId = history.replyToMessageId;
        msg.replySender = history.replySender;
        msg.replyPreview = history.replyPreview;
        msg.isForwarded = history.isForwarded;
        msg.forwardedFrom = history.forwardedFrom;
        msg.reactions = history.reactions;
        msg.mentions = history.mentions;

        if (history.senderId.trimmed().isEmpty()) {
            msg.senderId = QString();
            msg.sender = history.displayName.isEmpty() ? QStringLiteral("User") : history.displayName;
            msg.avatarPath = history.avatarPath;
            msg.isMine = history.isMine;
        } else {
            ParticipantInfo info = m_participants.value(history.senderId);
            const QString oldName = info.displayName;
            const QString oldAvatar = info.avatarPath;
            info.id = history.senderId;
            if (!history.displayName.isEmpty()) {
                info.displayName = history.displayName;
            }
            if (!history.avatarPath.isEmpty()) {
                info.avatarPath = history.avatarPath;
            }
            m_participants.insert(history.senderId, info);
            if ((!history.displayName.isEmpty() && oldName != info.displayName) ||
                (!history.avatarPath.isEmpty() && oldAvatar != info.avatarPath)) {
                updatedParticipants.insert(history.senderId, info);
            }

            msg.senderId = history.senderId;
            msg.sender = info.displayName.isEmpty() ? history.senderId : info.displayName;
            msg.avatarPath = info.avatarPath;
            msg.isMine = !m_currentUserId.isEmpty() && history.senderId == m_currentUserId;
            if (m_currentUserId.isEmpty()) {
                msg.isMine = history.isMine;
            }
        }

        if (!msg.messageId.isEmpty()) {
            m_messageIds.insert(msg.messageId);
        }
        converted.append(msg);
    }

    if (m_viewWidget) {
        m_viewWidget->appendMessages(converted);
    }
    if (!updatedParticipants.isEmpty()) {
        for (auto it = updatedParticipants.constBegin(); it != updatedParticipants.constEnd(); ++it) {
            const ParticipantInfo& info = it.value();
            const QString finalName = info.displayName.isEmpty() ? it.key() : info.displayName;
            if (auto* dataModel = model()) {
                dataModel->updateParticipantInfo(it.key(), finalName, info.avatarPath);
            }
        }
        if (m_viewWidget) {
            m_viewWidget->refreshLayout();
        }
    }
}

void ChatWidget::prependHistoryMessages(const QList<HistoryMessage>& messages, bool sortAndDedupe)
{
    if (messages.isEmpty()) {
        return;
    }
    QList<HistoryMessage> sorted = messages;
    if (sortAndDedupe) {
        std::sort(sorted.begin(), sorted.end(), [](const HistoryMessage& a, const HistoryMessage& b) {
            const qint64 at = a.timestamp.isValid() ? a.timestamp.toMSecsSinceEpoch() : 0;
            const qint64 bt = b.timestamp.isValid() ? b.timestamp.toMSecsSinceEpoch() : 0;
            return at < bt;
        });
    }
    QList<ChatWidgetMessage> converted;
    converted.reserve(sorted.size());
    QHash<QString, ParticipantInfo> updatedParticipants;
    for (const HistoryMessage& history : sorted) {
        if (sortAndDedupe && !history.messageId.isEmpty() && m_messageIds.contains(history.messageId)) {
            continue;
        }

        ChatWidgetMessage msg;
        msg.messageId = history.messageId;
        msg.content = history.content;
        msg.timestamp = history.timestamp;
        msg.messageType = history.messageType;
        msg.status = history.status;
        msg.imagePath = history.imagePath;
        msg.filePath = history.filePath;
        msg.fileName = history.fileName;
        msg.fileSize = history.fileSize;
        msg.replyToMessageId = history.replyToMessageId;
        msg.replySender = history.replySender;
        msg.replyPreview = history.replyPreview;
        msg.isForwarded = history.isForwarded;
        msg.forwardedFrom = history.forwardedFrom;
        msg.reactions = history.reactions;
        msg.mentions = history.mentions;

        if (history.senderId.trimmed().isEmpty()) {
            msg.senderId = QString();
            msg.sender = history.displayName.isEmpty() ? QStringLiteral("User") : history.displayName;
            msg.avatarPath = history.avatarPath;
            msg.isMine = history.isMine;
        } else {
            ParticipantInfo info = m_participants.value(history.senderId);
            const QString oldName = info.displayName;
            const QString oldAvatar = info.avatarPath;
            info.id = history.senderId;
            if (!history.displayName.isEmpty()) {
                info.displayName = history.displayName;
            }
            if (!history.avatarPath.isEmpty()) {
                info.avatarPath = history.avatarPath;
            }
            m_participants.insert(history.senderId, info);
            if ((!history.displayName.isEmpty() && oldName != info.displayName) ||
                (!history.avatarPath.isEmpty() && oldAvatar != info.avatarPath)) {
                updatedParticipants.insert(history.senderId, info);
            }

            msg.senderId = history.senderId;
            msg.sender = info.displayName.isEmpty() ? history.senderId : info.displayName;
            msg.avatarPath = info.avatarPath;
            msg.isMine = !m_currentUserId.isEmpty() && history.senderId == m_currentUserId;
            if (m_currentUserId.isEmpty()) {
                msg.isMine = history.isMine;
            }
        }

        if (!msg.messageId.isEmpty()) {
            m_messageIds.insert(msg.messageId);
        }
        converted.append(msg);
    }

    if (m_viewWidget) {
        m_viewWidget->prependMessages(converted);
    }
    if (!updatedParticipants.isEmpty()) {
        for (auto it = updatedParticipants.constBegin(); it != updatedParticipants.constEnd(); ++it) {
            const ParticipantInfo& info = it.value();
            const QString finalName = info.displayName.isEmpty() ? it.key() : info.displayName;
            if (auto* dataModel = model()) {
                dataModel->updateParticipantInfo(it.key(), finalName, info.avatarPath);
            }
        }
        if (m_viewWidget) {
            m_viewWidget->refreshLayout();
        }
    }
}

void ChatWidget::onStreamingTimeout()
{
    if (m_streamingIndex < m_streamingContent.length()) {
        int chunk = qMin(3, (int)m_streamingContent.length() - m_streamingIndex);
        streamOutput(m_streamingContent.mid(m_streamingIndex, chunk));
        m_streamingIndex += chunk;
    } else {
        setSendingState(false);
    }
}
