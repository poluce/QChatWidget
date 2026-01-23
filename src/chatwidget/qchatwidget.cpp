#include "qchatwidget.h"
#include "chatviewwidget.h"
#include "chatinputwidget.h"
#include <QVBoxLayout>

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
    m_viewWidget = new ChatViewWidget(this);
    m_inputWidget = new ChatInputWidget(this);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_viewWidget);
    m_mainLayout->addWidget(m_inputWidget);

    connect(m_inputWidget, &ChatInputWidgetBase::messageSent, this, &QChatWidget::onInputMessageSent);
    if (auto *input = qobject_cast<ChatInputWidget *>(m_inputWidget)) {
        connect(input, &ChatInputWidget::stopRequested, this, &QChatWidget::stopRequested);
    }
}

void QChatWidget::addMessage(const QString &content, bool isMine, const QString &sender)
{
    m_viewWidget->addMessage(content, isMine, sender);
}

void QChatWidget::streamOutput(const QString &content)
{
    m_viewWidget->streamOutput(content);
}

void QChatWidget::setInputWidget(ChatInputWidgetBase *widget)
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
    connect(m_inputWidget, &ChatInputWidgetBase::messageSent, this, &QChatWidget::onInputMessageSent);
    if (auto *input = qobject_cast<ChatInputWidget *>(m_inputWidget)) {
        connect(input, &ChatInputWidget::stopRequested, this, &QChatWidget::stopRequested);
    }
}

ChatInputWidgetBase *QChatWidget::inputWidget() const
{
    return m_inputWidget;
}

void QChatWidget::onInputMessageSent(const QString &content)
{
    addMessage(content, true, "Me");
    emit messageSent(content);
}
