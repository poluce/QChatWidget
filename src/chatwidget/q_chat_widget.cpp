#include "q_chat_widget.h"
#include "chat_widget_view.h"
#include "chat_widget_input.h"
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
    m_viewWidget = new ChatWidgetView(this);
    m_inputWidget = new ChatWidgetInput(this);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_viewWidget);
    m_mainLayout->addWidget(m_inputWidget);

    connect(m_inputWidget, &ChatWidgetInputBase::messageSent, this, &QChatWidget::onInputMessageSent);
    if (auto *input = qobject_cast<ChatWidgetInput *>(m_inputWidget)) {
        connect(input, &ChatWidgetInput::stopRequested, this, &QChatWidget::stopRequested);
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

void QChatWidget::setInputWidget(ChatWidgetInputBase *widget)
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
    connect(m_inputWidget, &ChatWidgetInputBase::messageSent, this, &QChatWidget::onInputMessageSent);
    if (auto *input = qobject_cast<ChatWidgetInput *>(m_inputWidget)) {
        connect(input, &ChatWidgetInput::stopRequested, this, &QChatWidget::stopRequested);
    }
}

ChatWidgetInputBase *QChatWidget::inputWidget() const
{
    return m_inputWidget;
}

void QChatWidget::onInputMessageSent(const QString &content)
{
    addMessage(content, true, "Me");
    emit messageSent(content);
}
