#include "chatviewwidget.h"
#include "chatmodel.h"
#include "chatdelegate.h"
#include <QListView>
#include <QVBoxLayout>
#include <QDateTime>

ChatViewWidget::ChatViewWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

ChatViewWidget::~ChatViewWidget()
{
}

void ChatViewWidget::setupUi()
{
    m_model = new ChatModel(this);
    m_delegate = new ChatDelegate(this);

    m_chatView = new QListView(this);
    m_chatView->setModel(m_model);
    m_chatView->setItemDelegate(m_delegate);
    m_chatView->setSelectionMode(QAbstractItemView::NoSelection);
    m_chatView->setFocusPolicy(Qt::NoFocus);
    m_chatView->setStyleSheet("QListView { background-color: #f5f5f5; border: none; }");
    m_chatView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_chatView->setResizeMode(QListView::Adjust);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chatView);
}

void ChatViewWidget::addMessage(const QString &content, bool isMine, const QString &sender)
{
    ChatMessage msg;
    msg.content = content;
    msg.isMine = isMine;
    msg.sender = sender;
    msg.timestamp = QDateTime::currentDateTime();

    if (!isMine) {
        msg.avatarPath = ":/res/other.png";
    }

    m_model->addMessage(msg);
    m_chatView->scrollToBottom();
}

void ChatViewWidget::streamOutput(const QString &content)
{
    m_model->appendContentToLastMessage(content);
    m_chatView->scrollToBottom();
}
