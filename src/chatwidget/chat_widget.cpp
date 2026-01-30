#include "chat_widget.h"
#include "chat_widget_input.h"
#include "chat_widget_model.h"
#include "chat_widget_view.h"
#include "qss_utils.h"
#include <QTimer>
#include <QVBoxLayout>

ChatWidget::ChatWidget(QWidget* parent) : QWidget(parent)
{
    m_streamingTimer = new QTimer(this);
    connect(m_streamingTimer, &QTimer::timeout, this, &ChatWidget::onStreamingTimeout);
    setupUi();
}

ChatWidget::~ChatWidget() { }

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
    connect(m_inputWidget, &ChatWidgetInputBase::messageSent, this, &ChatWidget::onInputMessageSent);
    connect(m_inputWidget, &ChatWidgetInputBase::stopRequested, this, [this]() {
    setSendingState(false);
    emit stopRequested(); });
}

void ChatWidget::addMessage(const QString& content, bool isMine, const QString& sender)
{
    m_viewWidget->addMessage(content, isMine, sender);
}

void ChatWidget::addMessage(const QString& content, const QString& senderId)
{
    addMessage(content, senderId, QString(), QString());
}

void ChatWidget::addMessage(const QString& content, const QString& senderId, const QString& displayName, const QString& avatarPath)
{
    if (senderId.trimmed().isEmpty()) {
        const QString fallbackName = displayName.isEmpty() ? QStringLiteral("User") : displayName;
        addMessage(content, false, fallbackName);
        return;
    }

    ParticipantInfo info = m_participants.value(senderId);
    info.id = senderId;
    if (!displayName.isEmpty()) {
        info.displayName = displayName;
    }
    if (!avatarPath.isEmpty()) {
        info.avatarPath = avatarPath;
    }
    m_participants.insert(senderId, info);

    const QString finalName = info.displayName.isEmpty() ? senderId : info.displayName;
    const bool isMine = !m_currentUserId.isEmpty() && senderId == m_currentUserId;
    m_viewWidget->addMessage(content, isMine, finalName, senderId, info.avatarPath);
}

void ChatWidget::streamOutput(const QString& content)
{
    // 物理拦截：如果当前不处于发送/接收状态，则拒绝任何字符追加
    if (!m_isSending) {
        return;
    }
    m_viewWidget->streamOutput(content);
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
    if (!m_currentUserId.isEmpty()) {
        addMessage(content, m_currentUserId);
    } else {
        addMessage(content, true, "Me");
    }
    emit messageSent(content);
}

void ChatWidget::removeLastMessage() { m_viewWidget->removeLastMessage(); }

void ChatWidget::clearMessages() { m_viewWidget->clearMessages(); }

int ChatWidget::messageCount() const { return m_viewWidget->messageCount(); }

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
    addMessage("", false, "AI");
    setSendingState(true);
    m_streamingTimer->start(interval);
}

void ChatWidget::setCurrentUserId(const QString& userId)
{
    if (m_currentUserId == userId) {
        return;
    }
    m_currentUserId = userId;
    if (m_viewWidget) {
        m_viewWidget->updateIsMine(m_currentUserId);
    }
}

QString ChatWidget::currentUserId() const
{
    return m_currentUserId;
}

void ChatWidget::setCurrentUser(const QString& userId, const QString& displayName, const QString& avatarPath)
{
    if (!userId.trimmed().isEmpty()) {
        upsertParticipant(userId, displayName, avatarPath);
    }
    setCurrentUserId(userId);
}

void ChatWidget::upsertParticipant(const QString& userId, const QString& displayName, const QString& avatarPath)
{
    ParticipantInfo info;
    info.id = userId;
    info.displayName = displayName;
    info.avatarPath = avatarPath;
    m_participants.insert(userId, info);
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
    }

    QList<ChatWidgetMessage> converted;
    converted.reserve(messages.size());
    for (const HistoryMessage& history : messages) {
        ChatWidgetMessage msg;
        msg.content = history.content;
        msg.timestamp = history.timestamp;

        if (history.senderId.trimmed().isEmpty()) {
            msg.senderId = QString();
            msg.sender = history.displayName.isEmpty() ? QStringLiteral("User") : history.displayName;
            msg.avatarPath = history.avatarPath;
            msg.isMine = history.isMine;
            converted.append(msg);
            continue;
        }

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
        converted.append(msg);
    }

    m_viewWidget->setMessages(converted);
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
