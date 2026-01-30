#include "chat_widget_delegate.h"
#include "chat_widget_markdown_utils.h"
#include "chat_widget_model.h"
#include <QAbstractTextDocumentLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QTextDocument>
#include <QVariant>
#include <QtMath>

namespace {
const int kLineSpacing = 6;
const int kReplyPadding = 6;
const int kReactionPaddingH = 8;
const int kReactionPaddingV = 2;
const int kAttachmentWidth = 180;
const int kAttachmentHeight = 120;
const int kFileCardHeight = 56;
const int kSystemPaddingH = 16;
const int kSystemPaddingV = 6;

QString formatStatus(ChatWidgetMessage::MessageStatus status)
{
    switch (status) {
    case ChatWidgetMessage::MessageStatus::Sending:
        return QStringLiteral("发送中");
    case ChatWidgetMessage::MessageStatus::Sent:
        return QStringLiteral("已发送");
    case ChatWidgetMessage::MessageStatus::Failed:
        return QStringLiteral("失败");
    case ChatWidgetMessage::MessageStatus::Read:
        return QStringLiteral("已读");
    default:
        return QString();
    }
}

QString formatTimestamp(const QDateTime& timestamp)
{
    if (!timestamp.isValid()) {
        return QString();
    }
    return timestamp.toString("HH:mm");
}

QString applyHighlights(const QString& html, const QStringList& mentions, const QString& keyword,
                        const ChatWidgetDelegate::Style& style)
{
    QString highlighted = html;
    const QString mentionColor = style.mentionHighlightColor.name();
    const QString searchColor = style.searchHighlightColor.name();

    for (const QString& mention : mentions) {
        if (mention.trimmed().isEmpty()) {
            continue;
        }
        const QString replacement =
            QString("<span style=\"background-color:%1;\">%2</span>").arg(mentionColor, mention);
        highlighted.replace(mention, replacement, Qt::CaseSensitive);
    }

    if (!keyword.trimmed().isEmpty()) {
        const QString replacement =
            QString("<span style=\"background-color:%1;\">%2</span>").arg(searchColor, keyword);
        highlighted.replace(keyword, replacement, Qt::CaseInsensitive);
    }

    return highlighted;
}

QList<ChatWidgetReaction> reactionsFromVariant(const QVariant& value)
{
    QList<ChatWidgetReaction> reactions;
    const QVariantList list = value.toList();
    reactions.reserve(list.size());
    for (const QVariant& item : list) {
        const QVariantMap map = item.toMap();
        const QString emoji = map.value("emoji").toString();
        if (emoji.isEmpty()) {
            continue;
        }
        ChatWidgetReaction reaction;
        reaction.emoji = emoji;
        reaction.count = map.value("count").toInt();
        reactions.append(reaction);
    }
    return reactions;
}

bool isSystemType(ChatWidgetMessage::MessageType type)
{
    return type == ChatWidgetMessage::MessageType::System ||
           type == ChatWidgetMessage::MessageType::DateSeparator;
}
} // namespace

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
    const auto type = static_cast<ChatWidgetMessage::MessageType>(
        index.data(ChatWidgetModel::ChatWidgetMessageTypeRole).toInt());
    const QString content = index.data(ChatWidgetModel::ChatWidgetContentRole).toString();
    const QDateTime timestamp = index.data(ChatWidgetModel::ChatWidgetTimestampRole).toDateTime();
    if (isSystemType(type)) {
        const QString text = content.isEmpty() && timestamp.isValid()
            ? timestamp.toString("yyyy-MM-dd")
            : content;
        QFontMetrics sysMetrics(m_style.systemFont);
        const int textWidth = sysMetrics.horizontalAdvance(text);
        const int maxWidth = option.rect.width() > 0 ? option.rect.width() * 0.8 : 400;
        const int finalWidth = qMin(maxWidth, textWidth + kSystemPaddingH * 2);
        const int height = sysMetrics.height() + kSystemPaddingV * 2 + m_style.margin * 2;
        return QSize(finalWidth, height);
    }

    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);
    const QStringList mentions = index.data(ChatWidgetModel::ChatWidgetMentionsRole).toStringList();
    const QString searchKeyword = index.data(ChatWidgetModel::ChatWidgetSearchKeywordRole).toString();
    html = applyHighlights(html, mentions, searchKeyword, m_style);

    int maxWidth = option.rect.width() * 0.6;
    if (maxWidth <= 0)
        maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(m_style.messageFont);
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    const int docHeight = qCeil(doc.size().height());

    const bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
    const QString senderName = index.data(ChatWidgetModel::ChatWidgetSenderRole).toString();
    const QString senderId = index.data(ChatWidgetModel::ChatWidgetSenderIdRole).toString();
    const QString imagePath = index.data(ChatWidgetModel::ChatWidgetImagePathRole).toString();
    const QString fileName = index.data(ChatWidgetModel::ChatWidgetFileNameRole).toString();
    const bool hasImage = !imagePath.isEmpty() || type == ChatWidgetMessage::MessageType::Image;
    const bool hasFile = !fileName.isEmpty() || type == ChatWidgetMessage::MessageType::File;

    const QString replySender = index.data(ChatWidgetModel::ChatWidgetReplySenderRole).toString();
    const QString replyPreview = index.data(ChatWidgetModel::ChatWidgetReplyPreviewRole).toString();
    const QString replyId = index.data(ChatWidgetModel::ChatWidgetReplyToMessageIdRole).toString();
    const bool isForwarded = index.data(ChatWidgetModel::ChatWidgetIsForwardedRole).toBool();
    const QString forwardedFrom = index.data(ChatWidgetModel::ChatWidgetForwardedFromRole).toString();
    const bool hasReply = !replySender.isEmpty() || !replyPreview.isEmpty() || !replyId.isEmpty();
    const bool hasForward = isForwarded || !forwardedFrom.isEmpty();

    int attachmentHeight = 0;
    if (hasImage) {
        attachmentHeight = kAttachmentHeight;
    } else if (hasFile) {
        attachmentHeight = kFileCardHeight;
    }

    int replyHeight = 0;
    if (hasReply || hasForward) {
        QFontMetrics replyMetrics(m_style.replyFont);
        int lines = 0;
        if (hasForward) {
            lines += 1;
        }
        if (hasReply) {
            lines += 1;
        }
        replyHeight = lines * replyMetrics.height() + kReplyPadding * 2;
    }

    const QList<ChatWidgetReaction> reactions =
        reactionsFromVariant(index.data(ChatWidgetModel::ChatWidgetReactionsRole));
    int reactionHeight = 0;
    if (!reactions.isEmpty()) {
        QFontMetrics reactionMetrics(m_style.reactionFont);
        reactionHeight = reactionMetrics.height() + kReactionPaddingV * 2;
    }

    int contentHeight = docHeight;
    if (replyHeight > 0) {
        contentHeight += replyHeight + kLineSpacing;
    }
    if (attachmentHeight > 0) {
        contentHeight += attachmentHeight + kLineSpacing;
    }
    if (reactionHeight > 0) {
        contentHeight += reactionHeight + kLineSpacing;
    }

    int totalHeight = contentHeight + (m_style.bubblePadding * 2) + (m_style.margin * 2);
    if (!isMine && !senderId.isEmpty() && !senderName.isEmpty()) {
        QFontMetrics nameMetrics(m_style.nameFont);
        totalHeight += nameMetrics.height() + m_style.nameSpacing;
    }

    const QString timestampText = formatTimestamp(timestamp);
    if (!timestampText.isEmpty() || isMine) {
        QFontMetrics timestampMetrics(m_style.timestampFont);
        QFontMetrics statusMetrics(m_style.statusFont);
        totalHeight += qMax(timestampMetrics.height(), statusMetrics.height()) + kLineSpacing;
    }

    return QSize(option.rect.width(), qMax(totalHeight, m_style.avatarSize + m_style.margin * 2));
}

void ChatWidgetDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const auto type = static_cast<ChatWidgetMessage::MessageType>(
        index.data(ChatWidgetModel::ChatWidgetMessageTypeRole).toInt());
    const bool isMine = index.data(ChatWidgetModel::ChatWidgetIsMineRole).toBool();
    const QString content = index.data(ChatWidgetModel::ChatWidgetContentRole).toString();
    const QDateTime timestamp = index.data(ChatWidgetModel::ChatWidgetTimestampRole).toDateTime();

    if (isSystemType(type)) {
        const QString text = content.isEmpty() && timestamp.isValid()
            ? timestamp.toString("yyyy-MM-dd")
            : content;
        QFontMetrics sysMetrics(m_style.systemFont);
        const int textWidth = sysMetrics.horizontalAdvance(text);
        const int maxWidth = option.rect.width() > 0 ? option.rect.width() * 0.8 : 400;
        const int bubbleWidth = qMin(maxWidth, textWidth + kSystemPaddingH * 2);
        const int bubbleHeight = sysMetrics.height() + kSystemPaddingV * 2;
        const int centerX = option.rect.center().x() - bubbleWidth / 2;
        const int centerY = option.rect.center().y() - bubbleHeight / 2;
        QRect bubbleRect(centerX, centerY, bubbleWidth, bubbleHeight);
        painter->setPen(Qt::NoPen);
        painter->setBrush(m_style.systemBubbleColor);
        painter->drawRoundedRect(bubbleRect, 10, 10);
        painter->setPen(m_style.systemTextColor);
        painter->setFont(m_style.systemFont);
        painter->drawText(bubbleRect, Qt::AlignCenter, text);
        painter->restore();
        return;
    }

    QString html = ChatWidgetMarkdownUtils::renderMarkdown(content);
    const QStringList mentions = index.data(ChatWidgetModel::ChatWidgetMentionsRole).toStringList();
    const QString searchKeyword = index.data(ChatWidgetModel::ChatWidgetSearchKeywordRole).toString();
    html = applyHighlights(html, mentions, searchKeyword, m_style);

    QRect rect = option.rect;
    int maxWidth = rect.width() * 0.6;
    if (maxWidth <= 0)
        maxWidth = 400;

    QTextDocument doc;
    doc.setDefaultFont(m_style.messageFont);
    doc.setHtml(html);
    doc.setTextWidth(maxWidth);

    const int docWidth = qMin(maxWidth, qCeil(doc.idealWidth()));
    const int docHeight = qCeil(doc.size().height());
    const QSize docSize(docWidth, docHeight);

    const QString imagePath = index.data(ChatWidgetModel::ChatWidgetImagePathRole).toString();
    const QString fileName = index.data(ChatWidgetModel::ChatWidgetFileNameRole).toString();
    const qint64 fileSize = index.data(ChatWidgetModel::ChatWidgetFileSizeRole).toLongLong();
    const bool hasImage = !imagePath.isEmpty() || type == ChatWidgetMessage::MessageType::Image;
    const bool hasFile = !fileName.isEmpty() || type == ChatWidgetMessage::MessageType::File;

    const QString replySender = index.data(ChatWidgetModel::ChatWidgetReplySenderRole).toString();
    const QString replyPreview = index.data(ChatWidgetModel::ChatWidgetReplyPreviewRole).toString();
    const QString replyId = index.data(ChatWidgetModel::ChatWidgetReplyToMessageIdRole).toString();
    const bool isForwarded = index.data(ChatWidgetModel::ChatWidgetIsForwardedRole).toBool();
    const QString forwardedFrom = index.data(ChatWidgetModel::ChatWidgetForwardedFromRole).toString();
    const bool hasReply = !replySender.isEmpty() || !replyPreview.isEmpty() || !replyId.isEmpty();
    const bool hasForward = isForwarded || !forwardedFrom.isEmpty();

    int attachmentWidth = 0;
    int attachmentHeight = 0;
    if (hasImage) {
        attachmentWidth = kAttachmentWidth;
        attachmentHeight = kAttachmentHeight;
    } else if (hasFile) {
        attachmentWidth = qMax(160, docWidth);
        attachmentHeight = kFileCardHeight;
    }

    int replyHeight = 0;
    if (hasReply || hasForward) {
        QFontMetrics replyMetrics(m_style.replyFont);
        int lines = 0;
        if (hasForward) {
            lines += 1;
        }
        if (hasReply) {
            lines += 1;
        }
        replyHeight = lines * replyMetrics.height() + kReplyPadding * 2;
    }

    const QList<ChatWidgetReaction> reactions =
        reactionsFromVariant(index.data(ChatWidgetModel::ChatWidgetReactionsRole));
    int reactionHeight = 0;
    if (!reactions.isEmpty()) {
        QFontMetrics reactionMetrics(m_style.reactionFont);
        reactionHeight = reactionMetrics.height() + kReactionPaddingV * 2;
    }

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
    const int contentWidth = qMax(docSize.width(), attachmentWidth);
    int contentHeight = docSize.height();
    if (replyHeight > 0) {
        contentHeight += replyHeight + kLineSpacing;
    }
    if (attachmentHeight > 0) {
        contentHeight += attachmentHeight + kLineSpacing;
    }
    if (reactionHeight > 0) {
        contentHeight += reactionHeight + kLineSpacing;
    }

    int bubbleWidth = contentWidth + m_style.bubblePadding * 2;
    int bubbleHeight = contentHeight + m_style.bubblePadding * 2;

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

    if (option.state & QStyle::State_Selected) {
        QPen selectionPen(m_style.selectionBorderColor);
        selectionPen.setWidth(1);
        painter->setPen(selectionPen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(bubbleRect.adjusted(1, 1, -1, -1), m_style.bubbleRadius, m_style.bubbleRadius);
    }

    QRect innerRect = bubbleRect.adjusted(m_style.bubblePadding, m_style.bubblePadding,
                                          -m_style.bubblePadding, -m_style.bubblePadding);
    int cursorY = innerRect.top();

    if (replyHeight > 0) {
        QRect replyRect(innerRect.left(), cursorY, innerRect.width(), replyHeight);
        painter->setPen(QPen(m_style.replyBorderColor));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(replyRect, 6, 6);
        painter->setPen(m_style.replyTextColor);
        painter->setFont(m_style.replyFont);
        int textY = replyRect.top() + kReplyPadding;
        QFontMetrics replyMetrics(m_style.replyFont);
        const int textWidth = replyRect.width() - kReplyPadding * 2;
        if (hasForward) {
            const QString forwardLabel = forwardedFrom.isEmpty()
                ? QStringLiteral("转发")
                : QStringLiteral("转发自 %1").arg(forwardedFrom);
            painter->drawText(QRect(replyRect.left() + kReplyPadding, textY, textWidth, replyMetrics.height()),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              replyMetrics.elidedText(forwardLabel, Qt::ElideRight, textWidth));
            textY += replyMetrics.height();
        }
        if (hasReply) {
            QString replyText;
            if (!replySender.isEmpty()) {
                replyText = QStringLiteral("回复 %1: %2").arg(replySender, replyPreview);
            } else {
                replyText = QStringLiteral("回复: %1").arg(replyPreview);
            }
            painter->drawText(QRect(replyRect.left() + kReplyPadding, textY, textWidth, replyMetrics.height()),
                              Qt::AlignLeft | Qt::AlignVCenter,
                              replyMetrics.elidedText(replyText, Qt::ElideRight, textWidth));
        }
        cursorY += replyHeight + kLineSpacing;
    }

    if (attachmentHeight > 0) {
        QRect attachRect(innerRect.left(), cursorY, qMin(innerRect.width(), attachmentWidth), attachmentHeight);
        painter->setPen(QPen(m_style.fileBorderColor));
        painter->setBrush(m_style.fileCardColor);
        painter->drawRoundedRect(attachRect, 6, 6);

        if (hasImage) {
            QPixmap image(imagePath);
            if (!image.isNull()) {
                QPainterPath clipPath;
                clipPath.addRoundedRect(attachRect, 6, 6);
                painter->save();
                painter->setClipPath(clipPath);
                painter->drawPixmap(attachRect, image);
                painter->restore();
            } else {
                painter->setPen(m_style.replyTextColor);
                painter->setFont(m_style.replyFont);
                painter->drawText(attachRect, Qt::AlignCenter, "Image");
            }
        } else if (hasFile) {
            painter->setPen(m_style.replyTextColor);
            painter->setFont(m_style.replyFont);
            QFontMetrics fileMetrics(m_style.replyFont);
            const int textWidth = attachRect.width() - kReplyPadding * 2;
            const QString nameText = fileMetrics.elidedText(fileName, Qt::ElideRight, textWidth);
            const QString sizeText = fileSize > 0
                ? QStringLiteral("%1 KB").arg(fileSize / 1024)
                : QStringLiteral("文件");
            painter->drawText(QRect(attachRect.left() + kReplyPadding, attachRect.top() + kReplyPadding,
                                    textWidth, fileMetrics.height()),
                              Qt::AlignLeft | Qt::AlignVCenter, nameText);
            painter->drawText(QRect(attachRect.left() + kReplyPadding,
                                    attachRect.bottom() - kReplyPadding - fileMetrics.height(),
                                    textWidth, fileMetrics.height()),
                              Qt::AlignLeft | Qt::AlignVCenter, sizeText);
        }
        cursorY += attachmentHeight + kLineSpacing;
    }

    if (docSize.height() > 0) {
        painter->save();
        painter->translate(innerRect.left(), cursorY);
        QRectF clip(0, 0, innerRect.width(), docSize.height());
        doc.drawContents(painter, clip);
        painter->restore();
        cursorY += docSize.height();
    }

    if (!reactions.isEmpty()) {
        cursorY += kLineSpacing;
        painter->setFont(m_style.reactionFont);
        QFontMetrics reactionMetrics(m_style.reactionFont);
        int x = innerRect.left();
        const int chipHeight = reactionMetrics.height() + kReactionPaddingV * 2;
        for (const ChatWidgetReaction& reaction : reactions) {
            const QString label = reaction.count > 0
                ? QString("%1 %2").arg(reaction.emoji).arg(reaction.count)
                : reaction.emoji;
            const int chipWidth = reactionMetrics.horizontalAdvance(label) + kReactionPaddingH * 2;
            QRect chipRect(x, cursorY, chipWidth, chipHeight);
            painter->setPen(Qt::NoPen);
            painter->setBrush(m_style.reactionChipColor);
            painter->drawRoundedRect(chipRect, chipHeight / 2, chipHeight / 2);
            painter->setPen(m_style.reactionTextColor);
            painter->drawText(chipRect, Qt::AlignCenter, label);
            x += chipWidth + 6;
            if (x + chipWidth > innerRect.right()) {
                break;
            }
        }
    }

    const QString timestampText = formatTimestamp(timestamp);
    const QString statusText = isMine ? formatStatus(static_cast<ChatWidgetMessage::MessageStatus>(
                                    index.data(ChatWidgetModel::ChatWidgetMessageStatusRole).toInt()))
                                      : QString();
    if (!timestampText.isEmpty() || !statusText.isEmpty()) {
        const int footerY = bubbleRect.bottom() + kLineSpacing;
        if (isMine) {
            painter->setFont(m_style.statusFont);
            QFontMetrics statusMetrics(m_style.statusFont);
            int textX = bubbleRect.right();
            if (!timestampText.isEmpty()) {
                painter->setFont(m_style.timestampFont);
                painter->setPen(m_style.timestampColor);
                QFontMetrics tsMetrics(m_style.timestampFont);
                const int tsWidth = tsMetrics.horizontalAdvance(timestampText);
                QRect tsRect(bubbleRect.right() - tsWidth, footerY, tsWidth, tsMetrics.height());
                painter->drawText(tsRect, Qt::AlignRight | Qt::AlignVCenter, timestampText);
                textX = tsRect.left() - 6;
            }
            if (!statusText.isEmpty()) {
                painter->setFont(m_style.statusFont);
                painter->setPen(m_style.statusColor);
                const int statusWidth = statusMetrics.horizontalAdvance(statusText);
                QRect statusRect(textX - statusWidth, footerY, statusWidth, statusMetrics.height());
                painter->drawText(statusRect, Qt::AlignRight | Qt::AlignVCenter, statusText);
            }
        } else if (!timestampText.isEmpty()) {
            painter->setFont(m_style.timestampFont);
            painter->setPen(m_style.timestampColor);
            QFontMetrics tsMetrics(m_style.timestampFont);
            QRect tsRect(bubbleRect.left(), footerY, tsMetrics.horizontalAdvance(timestampText), tsMetrics.height());
            painter->drawText(tsRect, Qt::AlignLeft | Qt::AlignVCenter, timestampText);
        }
    }

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
