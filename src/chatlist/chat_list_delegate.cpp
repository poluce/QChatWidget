#include "chat_list_delegate.h"
#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>

ChatListDelegate::ChatListDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

void ChatListDelegate::setStyle(const Style &style)
{
    m_style = style;
}

ChatListDelegate::Style ChatListDelegate::style() const
{
    return m_style;
}

QSize ChatListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index);
    const QFontMetrics fmName(m_style.nameFont);
    const QFontMetrics fmMsg(m_style.messageFont);
    const int textSpacing = qMax(4, fmMsg.leading());
    const int textBlockHeight = fmName.height() + fmMsg.height() + textSpacing;
    const int contentHeight = qMax(m_style.avatarSize, textBlockHeight);
    const int minHeight = contentHeight + 2 * m_style.margin;
    const int height = (m_style.itemHeight > 0) ? qMax(m_style.itemHeight, minHeight) : minHeight;
    return QSize(option.rect.width(), height);
}

void ChatListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 1. 获取数据
    QString name = index.data(ChatListNameRole).toString();
    QString message = index.data(ChatListMessageRole).toString();
    QString time = index.data(ChatListTimeRole).toString();
    QColor avatarColor = index.data(ChatListAvatarColorRole).value<QColor>();
    int unreadCount = index.data(ChatListUnreadCountRole).toInt();

    // 2. 绘制背景
    QRect rect = option.rect;
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, m_style.selectedColor);
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, m_style.hoverColor);
    } else {
        painter->fillRect(rect, m_style.backgroundColor);
    }

    // 3. 布局参数
    int avatarSize = m_style.avatarSize;
    int margin = m_style.margin;
    int textLeftMargin = margin + avatarSize + margin;
    const QFontMetrics fmName(m_style.nameFont);
    const QFontMetrics fmMsg(m_style.messageFont);
    const QFontMetrics fmTime(m_style.timeFont);
    const int textSpacing = qMax(4, fmMsg.leading());
    const int textBlockHeight = fmName.height() + fmMsg.height() + textSpacing;
    const int contentHeight = qMax(avatarSize, textBlockHeight);
    const int contentTop = rect.top() + (rect.height() - contentHeight) / 2;

    // 4. 绘制头像
    QRect avatarRect(rect.left() + margin,
                     contentTop + (contentHeight - avatarSize) / 2,
                     avatarSize,
                     avatarSize);
    if (m_style.avatarShape == AvatarSquare) {
        painter->fillRect(avatarRect, avatarColor);
    } else {
        QPainterPath path;
        if (m_style.avatarShape == AvatarRoundedRect) {
            const int radius = qMax(0, m_style.avatarCornerRadius);
            path.addRoundedRect(avatarRect, radius, radius);
        } else {
            path.addEllipse(avatarRect);
        }
        painter->setClipPath(path);
        painter->fillRect(avatarRect, avatarColor);
        painter->setClipping(false);
    }

    // 5. 绘制未读红点
    if (m_style.showUnreadBadge && unreadCount > 0) {
        int badgeSize = m_style.badgeSize;
        QRect badgeRect(avatarRect.right() - 6, avatarRect.top() - 6, badgeSize, badgeSize);
        painter->setBrush(m_style.badgeColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badgeRect);
        
        painter->setPen(m_style.badgeTextColor);
        QFont badgeFont = m_style.badgeFont;
        painter->setFont(badgeFont);
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(unreadCount));
    }

    // 6. 绘制昵称
    painter->setPen(m_style.nameColor);
    QFont nameFont = m_style.nameFont;
    painter->setFont(nameFont);
    
    QFont timeFont = m_style.timeFont;
    int timeWidth = fmTime.horizontalAdvance(time) + margin;

    const int nameWidth = qMax(0, rect.width() - textLeftMargin - timeWidth - margin);
    QRect nameRect(rect.left() + textLeftMargin,
                   contentTop,
                   nameWidth,
                   fmName.height());
    QString elidedName = painter->fontMetrics().elidedText(name, Qt::ElideRight, nameRect.width());
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    // 7. 绘制时间
    painter->setPen(m_style.timeColor);
    painter->setFont(timeFont);
    QRect timeRect(rect.right() - timeWidth,
                   contentTop,
                   timeWidth - margin,
                   fmTime.height());
    painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, time);

    // 8. 绘制消息内容
    painter->setPen(m_style.messageColor);
    QFont msgFont = m_style.messageFont;
    painter->setFont(msgFont);

    const int msgWidth = qMax(0, rect.width() - textLeftMargin - margin);
    QRect msgRect(rect.left() + textLeftMargin,
                  contentTop + fmName.height() + textSpacing,
                  msgWidth,
                  fmMsg.height());
    QString elidedMsg = painter->fontMetrics().elidedText(message, Qt::ElideRight, msgRect.width());
    painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter, elidedMsg);

    // 9. 分隔线
    if (m_style.showSeparator) {
        painter->setPen(m_style.separatorColor);
        painter->drawLine(textLeftMargin, rect.bottom(), rect.right(), rect.bottom());
    }

    painter->restore();
}
