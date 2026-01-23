#include "chat_widget_delegate.h"
#include "chat_widget_model.h"
#include "chat_widget_markdown_utils.h"
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QDebug>

ChatWidgetDelegate::ChatWidgetDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize ChatWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatWidgetModel::ContentRole).toString();
    // Convert to HTML
    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);
    
    int maxWidth = option.rect.width() * 0.6;
    if (maxWidth <= 0) maxWidth = 400; // Fallback

    QTextDocument doc;
    doc.setDefaultFont(QFont("Arial", 11));
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);
    
    int docHeight = doc.size().height();
    int totalHeight = docHeight + (m_bubblePadding * 2) + (m_margin * 2);
    
    return QSize(option.rect.width(), qMax(totalHeight, m_avatarSize + m_margin * 2));
}

void ChatWidgetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool isMine = index.data(ChatWidgetModel::IsMineRole).toBool();
    QString content = index.data(ChatWidgetModel::ContentRole).toString();
    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);

    // Layout Logic
    QRect rect = option.rect;
    int maxWidth = rect.width() * 0.6;
    if (maxWidth <= 0) maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(QFont("Arial", 11));
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    QSize docSize(doc.idealWidth(), doc.size().height());
    
    // Draw Avatar
    QRect avatarRect;
    if (isMine) {
        avatarRect = QRect(rect.right() - m_margin - m_avatarSize, 
                           rect.top() + m_margin, 
                           m_avatarSize, m_avatarSize);
    } else {
        avatarRect = QRect(rect.left() + m_margin, 
                           rect.top() + m_margin, 
                           m_avatarSize, m_avatarSize);
    }
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(isMine ? QColor(0, 120, 215) : QColor(200, 200, 200));
    painter->drawEllipse(avatarRect);
    
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(avatarRect, Qt::AlignCenter, isMine ? "Me" : "U");

    // Draw Bubble
    QRect bubbleRect;
    int bubbleWidth = docSize.width() + m_bubblePadding * 2;
    int bubbleHeight = docSize.height() + m_bubblePadding * 2;

    if (isMine) {
        bubbleRect = QRect(avatarRect.left() - m_margin - bubbleWidth,
                           rect.top() + m_margin,
                           bubbleWidth,
                           bubbleHeight);
        painter->setBrush(QColor(158, 234, 106)); 
    } else {
        bubbleRect = QRect(avatarRect.right() + m_margin,
                           rect.top() + m_margin,
                           bubbleWidth,
                           bubbleHeight);
        painter->setBrush(Qt::white);
    }

    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, 6, 6);

    // Draw Text (Render Document)
    painter->translate(bubbleRect.left() + m_bubblePadding, bubbleRect.top() + m_bubblePadding);
    QRectF clip(0, 0, docSize.width(), docSize.height());
    doc.drawContents(painter, clip);

    painter->restore();
}
