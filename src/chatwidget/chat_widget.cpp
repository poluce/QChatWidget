#include "chat_widget.h"
#include "chat_widget_view.h"
#include "chat_widget_input.h"
#include <QVBoxLayout>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

ChatWidget::~ChatWidget()
{
}

void ChatWidget::setupUi()
{
    m_viewWidget = new ChatWidgetView(this);
    m_inputWidget = new ChatWidgetInput(this);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_viewWidget);
    m_mainLayout->addWidget(m_inputWidget);

    connect(m_inputWidget, &ChatWidgetInputBase::messageSent, this, &ChatWidget::onInputMessageSent);
    if (auto *input = qobject_cast<ChatWidgetInput *>(m_inputWidget)) {
        connect(input, &ChatWidgetInput::stopRequested, this, &ChatWidget::stopRequested);
    }
}

void ChatWidget::addMessage(const QString &content, bool isMine, const QString &sender)
{
    m_viewWidget->addMessage(content, isMine, sender);
}

void ChatWidget::streamOutput(const QString &content)
{
    m_viewWidget->streamOutput(content);
}

void ChatWidget::setInputWidget(ChatWidgetInputBase *widget)
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
    if (auto *input = qobject_cast<ChatWidgetInput *>(m_inputWidget)) {
        connect(input, &ChatWidgetInput::stopRequested, this, &ChatWidget::stopRequested);
    }
}

ChatWidgetInputBase *ChatWidget::inputWidget() const
{
    return m_inputWidget;
}

void ChatWidget::onInputMessageSent(const QString &content)
{
    addMessage(content, true, "Me");
    emit messageSent(content);
}
