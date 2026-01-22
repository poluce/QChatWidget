#include "qchatwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

QChatWidget::QChatWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

QChatWidget::~QChatWidget()
{
}

void QChatWidget::setupUi()
{
    // Initialize components
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

    m_inputEdit = new QLineEdit(this);
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setFixedWidth(60);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // No margin for the widget itself
    mainLayout->addWidget(m_chatView);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(5, 5, 5, 5); // Some margin for input area
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendButton);

    mainLayout->addLayout(inputLayout);

    // Connections
    connect(m_sendButton, &QPushButton::clicked, this, &QChatWidget::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &QChatWidget::onSendClicked);
}

void QChatWidget::addMessage(const QString &content, bool isMine, const QString &sender)
{
    ChatMessage msg;
    msg.content = content;
    msg.isMine = isMine;
    msg.sender = sender;
    msg.timestamp = QDateTime::currentDateTime();
    
    // 简单的头像逻辑预留，可按需扩展
    if (!isMine) {
        msg.avatarPath = ":/res/other.png"; 
    }

    m_model->addMessage(msg);
    m_chatView->scrollToBottom();
}

void QChatWidget::onSendClicked()
{
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    // 先在本地显示
    addMessage(text, true, "Me");

    // 发射信号供外部逻辑处理
    emit messageSent(text);

    m_inputEdit->clear();
}

void QChatWidget::streamOutput(const QString &content)
{
    m_model->appendContentToLastMessage(content);
    m_chatView->scrollToBottom();
}
