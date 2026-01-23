#include "chat_widget_view.h"
#include "chat_widget_model.h"
#include "chat_widget_delegate.h"
#include <QListView>
#include <QVBoxLayout>
#include <QDateTime>

ChatWidgetView::ChatWidgetView(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("chatWidgetView");
    setupUi();
}

ChatWidgetView::~ChatWidgetView()
{
}

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

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chatView);
}

void ChatWidgetView::addMessage(const QString &content, bool isMine, const QString &sender)
{
    ChatWidgetMessage msg;
    msg.content = content;
    msg.isMine = isMine;
    msg.sender = sender;
    msg.timestamp = QDateTime::currentDateTime();

    if (!isMine)
    {
        msg.avatarPath = ":/res/other.png";
    }

    m_model->addMessage(msg);
    m_chatView->scrollToBottom();
}

void ChatWidgetView::streamOutput(const QString &content)
{
    m_model->appendContentToLastMessage(content);
    m_chatView->scrollToBottom();
}

void ChatWidgetView::setDelegateStyle(const ChatWidgetDelegate::Style &style)
{
    m_delegate->setStyle(style);
    m_chatView->viewport()->update();
}

ChatWidgetDelegate::Style ChatWidgetView::delegateStyle() const
{
    return m_delegate->style();
}
