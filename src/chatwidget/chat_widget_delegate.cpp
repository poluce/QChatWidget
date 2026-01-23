#include "chat_widget_delegate.h"
#include "chat_widget_model.h"
#include "chat_widget_markdown_utils.h"
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

ChatWidgetDelegate::ChatWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ChatWidgetDelegate::setStyle(const Style &style)
{
    m_style = style;
}

ChatWidgetDelegate::Style ChatWidgetDelegate::style() const
{
    return m_style;
}

QSize ChatWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatWidgetModel::ChatWidgetContentRole).toString();
    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);

    int maxWidth = option.rect.width() * 0.6;
    if (maxWidth <= 0) maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(m_style.messageFont);
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    int docHeight = doc.size().height();
    int totalHeight = docHeight + (m_style.bubblePadding * 2) + (m_style.margin * 2);

    return QSize(option.rect.width(), qMax(totalHeight, m_style.avatarSize + m_style.margin * 2));
}

void ChatWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
    QString content = index.data(ChatWidgetModel::ChatWidgetContentRole).toString();
    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);

    QRect rect = option.rect;
    int maxWidth = rect.width() * 0.6;
    if (maxWidth <= 0) maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(m_style.messageFont);
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    QSize docSize(doc.idealWidth(), doc.size().height());

    // 绘制头像
    QRect avatarRect;
    if (isMine) {
        avatarRect = QRect(rect.right() - m_style.margin - m_style.avatarSize,
                           rect.top() + m_style.margin,
                           m_style.avatarSize, m_style.avatarSize);
    } else {
        avatarRect = QRect(rect.left() + m_style.margin,
                           rect.top() + m_style.margin,
                           m_style.avatarSize, m_style.avatarSize);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(isMine ? m_style.myAvatarColor : m_style.otherAvatarColor);
    painter->drawEllipse(avatarRect);

    painter->setPen(Qt::white);
    painter->setFont(m_style.avatarFont);
    painter->drawText(avatarRect, Qt::AlignCenter, isMine ? "Me" : "U");

    // 绘制气泡
    QRect bubbleRect;
    int bubbleWidth = docSize.width() + m_style.bubblePadding * 2;
    int bubbleHeight = docSize.height() + m_style.bubblePadding * 2;

    if (isMine) {
        bubbleRect = QRect(avatarRect.left() - m_style.margin - bubbleWidth,
                           rect.top() + m_style.margin,
                           bubbleWidth,
                           bubbleHeight);
        painter->setBrush(m_style.myBubbleColor);
    } else {
        bubbleRect = QRect(avatarRect.right() + m_style.margin,
                           rect.top() + m_style.margin,
                           bubbleWidth,
                           bubbleHeight);
        painter->setBrush(m_style.otherBubbleColor);
    }

    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, m_style.bubbleRadius, m_style.bubbleRadius);

    // 绘制文本
    painter->translate(bubbleRect.left() + m_style.bubblePadding, bubbleRect.top() + m_style.bubblePadding);
    QRectF clip(0, 0, docSize.width(), docSize.height());
    doc.drawContents(painter, clip);

    painter->restore();
}
