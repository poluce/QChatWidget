#include "chat_widget_view.h"
#include "chat_widget_delegate.h"
#include "chat_widget_model.h"
#include <QDateTime>
#include <QListView>
#include <QMouseEvent>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

ChatWidgetView::ChatWidgetView(QWidget* parent) : QWidget(parent)
{
    setObjectName("chatWidgetView");
    setupUi();
}

ChatWidgetView::~ChatWidgetView() { }

void ChatWidgetView::setupUi()
{
    m_model = new ChatWidgetModel(this);
    m_delegate = new ChatWidgetDelegate(this);

    m_chatView = new QListView(this);
    m_chatView->setObjectName("chatWidgetViewList");
    m_chatView->setModel(m_model);
    m_chatView->setItemDelegate(m_delegate);
    m_chatView->setObjectName("chatWidgetViewList");
    m_chatView->setSelectionMode(QAbstractItemView::NoSelection);
    m_chatView->setFocusPolicy(Qt::NoFocus);
    m_chatView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_chatView->setResizeMode(QListView::Adjust);
    m_chatView->viewport()->installEventFilter(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chatView);
}

void ChatWidgetView::addMessage(const QString& content, bool isMine, const QString& sender,
                                const QString& senderId, const QString& avatarPath)
{
    ChatWidgetMessage msg;
    msg.senderId = senderId;
    msg.content = content;
    msg.isMine = isMine;
    msg.sender = sender;
    msg.avatarPath = avatarPath;
    msg.timestamp = QDateTime::currentDateTime();

    m_model->addMessage(msg);
    m_chatView->scrollToBottom();
}

void ChatWidgetView::setMessages(const QList<ChatWidgetMessage>& messages)
{
    m_model->setMessages(messages);
    m_chatView->scrollToBottom();
}

void ChatWidgetView::appendMessages(const QList<ChatWidgetMessage>& messages)
{
    m_model->appendMessages(messages);
    m_chatView->scrollToBottom();
}

void ChatWidgetView::prependMessages(const QList<ChatWidgetMessage>& messages)
{
    m_model->prependMessages(messages);
    m_chatView->viewport()->update();
}

void ChatWidgetView::updateIsMine(const QString& currentUserId)
{
    m_model->updateIsMine(currentUserId);
    m_chatView->doItemsLayout();
    m_chatView->viewport()->update();
}

void ChatWidgetView::updateParticipantInfo(const QString& senderId, const QString& displayName, const QString& avatarPath)
{
    m_model->updateParticipantInfo(senderId, displayName, avatarPath);
    m_chatView->viewport()->update();
}

void ChatWidgetView::streamOutput(const QString& content)
{
    m_model->appendContentToLastMessage(content);
    m_chatView->scrollToBottom();
}

void ChatWidgetView::setDelegateStyle(const ChatWidgetDelegate::Style& style)
{
    m_delegate->setStyle(style);
    m_chatView->viewport()->update();
}

ChatWidgetDelegate::Style ChatWidgetView::delegateStyle() const
{
    return m_delegate->style();
}

void ChatWidgetView::removeLastMessage()
{
    m_model->removeLastMessage();
}

void ChatWidgetView::clearMessages()
{
    m_model->clearMessages();
}

int ChatWidgetView::messageCount() const
{
    return m_model->messageCount();
}

bool ChatWidgetView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_chatView->viewport() && event->type() == QEvent::MouseButtonRelease) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            const QModelIndex index = m_chatView->indexAt(mouseEvent->pos());
            if (index.isValid()) {
                QStyleOptionViewItem option = m_chatView->viewOptions();
                option.rect = m_chatView->visualRect(index);
                const QRect avatarHitRect = m_delegate->avatarRect(option, index);
                if (avatarHitRect.contains(mouseEvent->pos())) {
                    const QString sender = index.data(ChatWidgetModel::ChatWidgetSenderRole).toString();
                    const QString senderId = index.data(ChatWidgetModel::ChatWidgetSenderIdRole).toString();
                    const bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
                    emit avatarClicked(sender, isMine, index.row());
                    if (!senderId.isEmpty()) {
                        if (isMine) {
                            emit selfAvatarClicked(senderId, index.row());
                        } else {
                            emit memberAvatarClicked(senderId, sender, index.row());
                        }
                    }
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}
