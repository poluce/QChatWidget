#ifndef CHAT_LIST_DELEGATE_H
#define CHAT_LIST_DELEGATE_H

#include <QStyledItemDelegate>
#include <QColor>
#include <QFont>
#include "chat_list_roles.h"

class ChatListDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    enum AvatarShape {
        AvatarCircle,
        AvatarRoundedRect,
        AvatarSquare
    };

    struct Style {
        int itemHeight = 72;
        int avatarSize = 50;
        int margin = 12;
        int badgeSize = 18;
        int avatarCornerRadius = 8;
        AvatarShape avatarShape = AvatarCircle;

        QColor backgroundColor = Qt::white;
        QColor hoverColor = QColor(220, 220, 220);
        QColor selectedColor = QColor(195, 195, 195);
        QColor nameColor = QColor(25, 25, 25);
        QColor messageColor = QColor(150, 150, 150);
        QColor timeColor = QColor(180, 180, 180);
        QColor separatorColor = QColor(230, 230, 230);
        QColor badgeColor = Qt::red;
        QColor badgeTextColor = Qt::white;

        QFont nameFont = QFont("Arial", 16);
        QFont messageFont = QFont("Arial", 14);
        QFont timeFont = QFont("Arial", 12);
        QFont badgeFont = QFont("Arial", 10);

        bool showSeparator = true;
        bool showUnreadBadge = true;
    };

    explicit ChatListDelegate(QObject *parent = nullptr);

    void setStyle(const Style &style);
    Style style() const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    Style m_style;
};

#endif // CHAT_LIST_DELEGATE_H
