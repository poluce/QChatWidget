#include "chat_widget_delegate.h"
#include "chat_widget_markdown_utils.h"
#include "chat_widget_model.h"
#include <QAbstractTextDocumentLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QTextDocument>

ChatWidgetDelegate::ChatWidgetDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void ChatWidgetDelegate::setStyle(const Style& style)
{
    m_style = style;
}

ChatWidgetDelegate::Style ChatWidgetDelegate::style() const
{
    return m_style;
}

QSize ChatWidgetDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString content = index.data(ChatWidgetModel::ChatWidgetContentRole).toString();
    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);
    const bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
    const QString senderName = index.data(ChatWidgetModel::ChatWidgetSenderRole).toString();
    const QString senderId = index.data(ChatWidgetModel::ChatWidgetSenderIdRole).toString();

    int maxWidth = option.rect.width() * 0.6;
    if (maxWidth <= 0)
        maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(m_style.messageFont);
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    int docHeight = doc.size().height();
    int totalHeight = docHeight + (m_style.bubblePadding * 2) + (m_style.margin * 2);
    if (!isMine && !senderId.isEmpty() && !senderName.isEmpty()) {
        QFontMetrics nameMetrics(m_style.nameFont);
        totalHeight += nameMetrics.height() + m_style.nameSpacing;
    }

    return QSize(option.rect.width(), qMax(totalHeight, m_style.avatarSize + m_style.margin * 2));
}

void ChatWidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
    QString content = index.data(ChatWidgetModel::ChatWidgetContentRole).toString();
    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);

    QRect rect = option.rect;
    int maxWidth = rect.width() * 0.6;
    if (maxWidth <= 0)
        maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(m_style.messageFont);
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    QSize docSize(doc.idealWidth(), doc.size().height());

    // 绘制头像
    QRect avatarRect = this->avatarRect(option, index);
    const QString avatarPath = index.data(ChatWidgetModel::ChatWidgetAvatarRole).toString();
    const QString senderName = index.data(ChatWidgetModel::ChatWidgetSenderRole).toString();
    const QString senderId = index.data(ChatWidgetModel::ChatWidgetSenderIdRole).toString();

    painter->setPen(Qt::NoPen);
    bool drawAvatarText = true;
    if (!avatarPath.isEmpty()) {
        QPixmap avatarPixmap(avatarPath);
        if (!avatarPixmap.isNull()) {
            QPainterPath clipPath;
            clipPath.addEllipse(avatarRect);
            painter->setClipPath(clipPath);
            painter->drawPixmap(avatarRect, avatarPixmap);
            painter->setClipping(false);
            drawAvatarText = false;
        } else {
            painter->setBrush(isMine ? m_style.myAvatarColor : m_style.otherAvatarColor);
            painter->drawEllipse(avatarRect);
        }
    } else {
        painter->setBrush(isMine ? m_style.myAvatarColor : m_style.otherAvatarColor);
        painter->drawEllipse(avatarRect);
    }

    if (drawAvatarText) {
        painter->setPen(Qt::white);
        painter->setFont(m_style.avatarFont);
        const QString avatarText = senderName.isEmpty() ? (isMine ? "Me" : "U") : senderName.left(1);
        painter->drawText(avatarRect, Qt::AlignCenter, avatarText);
    }

    // 绘制气泡
    QRect bubbleRect;
    int bubbleWidth = docSize.width() + m_style.bubblePadding * 2;
    int bubbleHeight = docSize.height() + m_style.bubblePadding * 2;

    int contentTop = rect.top() + m_style.margin;
    if (!isMine && !senderId.isEmpty() && !senderName.isEmpty()) {
        painter->setPen(m_style.nameColor);
        painter->setFont(m_style.nameFont);
        QFontMetrics nameMetrics(m_style.nameFont);
        const int nameHeight = nameMetrics.height();
        QRect nameRect(avatarRect.right() + m_style.margin, contentTop,
                       rect.right() - avatarRect.right() - m_style.margin * 2, nameHeight);
        painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, senderName);
        contentTop += nameHeight + m_style.nameSpacing;
    }

    if (isMine) {
        bubbleRect = QRect(avatarRect.left() - m_style.margin - bubbleWidth, contentTop, bubbleWidth, bubbleHeight);
        painter->setBrush(m_style.myBubbleColor);
    } else {
        bubbleRect = QRect(avatarRect.right() + m_style.margin, contentTop, bubbleWidth, bubbleHeight);
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

QRect ChatWidgetDelegate::avatarRect(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
    const QRect rect = option.rect;
    if (isMine) {
        return QRect(rect.right() - m_style.margin - m_style.avatarSize, rect.top() + m_style.margin, m_style.avatarSize, m_style.avatarSize);
    }
    return QRect(rect.left() + m_style.margin, rect.top() + m_style.margin, m_style.avatarSize, m_style.avatarSize);
}
