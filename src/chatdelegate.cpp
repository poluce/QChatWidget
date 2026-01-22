#include "chatdelegate.h"
#include "chatmodel.h"
#include <QPainter>
#include <QTextOption>

ChatDelegate::ChatDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QSize ChatDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString content = index.data(ChatModel::ContentRole).toString();
    
    int maxWidth = option.rect.width() * 0.6;
    
    QFont font = option.font;
    font.setPointSize(11); 
    QFontMetrics fm(font);
    
    QRect textRect = fm.boundingRect(0, 0, maxWidth, 10000, 
                                     Qt::TextWordWrap | Qt::AlignLeft, content);
    
    int totalHeight = textRect.height() + (m_bubblePadding * 2) + (m_margin * 2);
    
    return QSize(option.rect.width(), qMax(totalHeight, m_avatarSize + m_margin * 2));
}

void ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    bool isMine = index.data(ChatModel::IsMineRole).toBool();
    QString content = index.data(ChatModel::ContentRole).toString();

    // 绘制头像
    QRect avatarRect;
    if (isMine) {
        avatarRect = QRect(option.rect.right() - m_margin - m_avatarSize, 
                           option.rect.top() + m_margin, 
                           m_avatarSize, m_avatarSize);
    } else {
        avatarRect = QRect(option.rect.left() + m_margin, 
                           option.rect.top() + m_margin, 
                           m_avatarSize, m_avatarSize);
    }
    
    painter->setPen(Qt::NoPen);
    painter->setBrush(isMine ? QColor(0, 120, 215) : QColor(200, 200, 200));
    painter->drawEllipse(avatarRect);
    
    painter->setPen(Qt::white);
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(avatarRect, Qt::AlignCenter, isMine ? "Me" : "U");

    // 绘制气泡
    QFont font = option.font;
    font.setPointSize(11);
    painter->setFont(font);
    QFontMetrics fm(font);
    
    int maxWidth = option.rect.width() * 0.6;
    QRect textBounding = fm.boundingRect(0, 0, maxWidth, 10000, 
                                         Qt::TextWordWrap | Qt::AlignLeft, content);
    
    QRect bubbleRect;
    if (isMine) {
        bubbleRect = QRect(avatarRect.left() - m_margin - textBounding.width() - m_bubblePadding * 2,
                           option.rect.top() + m_margin,
                           textBounding.width() + m_bubblePadding * 2,
                           textBounding.height() + m_bubblePadding * 2);
        painter->setBrush(QColor(158, 234, 106)); 
    } else {
        bubbleRect = QRect(avatarRect.right() + m_margin,
                           option.rect.top() + m_margin,
                           textBounding.width() + m_bubblePadding * 2,
                           textBounding.height() + m_bubblePadding * 2);
        painter->setBrush(Qt::white);
    }

    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(bubbleRect, 6, 6);

    // 绘制文字
    painter->setPen(Qt::black);
    QRect textDrawRect = bubbleRect.adjusted(m_bubblePadding, m_bubblePadding, -m_bubblePadding, -m_bubblePadding);
    painter->drawText(textDrawRect, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignVCenter, content);

    painter->restore();
}