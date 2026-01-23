#ifndef CHAT_WIDGET_DELEGATE_H
#define CHAT_WIDGET_DELEGATE_H

#include <QStyledItemDelegate>
#include <QColor>
#include <QFont>

class ChatWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    struct Style {
        int avatarSize = 40;
        int margin = 10;
        int bubblePadding = 10;
        int bubbleRadius = 8;

        QColor myBubbleColor = QColor(149, 236, 105);
        QColor otherBubbleColor = Qt::white;
        QColor myAvatarColor = QColor(0, 120, 215);
        QColor otherAvatarColor = QColor(200, 200, 200);
        QColor myTextColor = QColor(25, 25, 25);
        QColor otherTextColor = QColor(25, 25, 25);
        QColor backgroundColor = QColor(245, 245, 245);

        QFont messageFont = QFont("Microsoft YaHei", 11);
        QFont avatarFont = QFont("Microsoft YaHei", 10, QFont::Bold);
    };

    explicit ChatWidgetDelegate(QObject *parent = nullptr);

    void setStyle(const Style &style);
    Style style() const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    Style m_style;
};

#endif // CHAT_WIDGET_DELEGATE_H
