#include "chat_widget.h"
#include "chat_widget_view.h"
#include "chat_widget_input.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QVBoxLayout>

namespace {
QString resolveStyleSheetPath(const QString &fileNameOrPath)
{
    if (fileNameOrPath.startsWith(":/")) {
        return fileNameOrPath;
    }
    if (QFile::exists(fileNameOrPath)) {
        return QFileInfo(fileNameOrPath).absoluteFilePath();
    }

    const QString fileName = QFileInfo(fileNameOrPath).fileName();
    if (fileName.isEmpty()) {
        return QString();
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath("resources/styles/" + fileName),
        QDir(appDir).filePath("../resources/styles/" + fileName),
        QDir(QDir::currentPath()).filePath("resources/styles/" + fileName),
        QDir(QDir::currentPath()).filePath("../resources/styles/" + fileName)
    };

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }
    return QString();
}

bool applyStyleSheetFromFile(QWidget *target, const QString &fileNameOrPath)
{
    if (!target) {
        return false;
    }

    const QString resolved = resolveStyleSheetPath(fileNameOrPath);
    const QString fileName = QFileInfo(fileNameOrPath).fileName();
    QFile file(resolved.isEmpty() ? QString(":/styles/%1").arg(fileName) : resolved);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return false;
    }

    target->setStyleSheet(QString::fromUtf8(file.readAll()));
    return true;
}
} // namespace

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

bool ChatWidget::applyStyleSheetFile(const QString &fileNameOrPath)
{
    if (fileNameOrPath.trimmed().isEmpty()) {
        return false;
    }
    return applyStyleSheetFromFile(this, fileNameOrPath);
}

void ChatWidget::setDelegateStyle(const ChatWidgetDelegate::Style &style)
{
    m_viewWidget->setDelegateStyle(style);
}

ChatWidgetDelegate::Style ChatWidget::delegateStyle() const
{
    return m_viewWidget->delegateStyle();
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
