#ifndef CHAT_WIDGET_DELEGATE_H
#define CHAT_WIDGET_DELEGATE_H

#include <QStyledItemDelegate>

class ChatWidgetDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatWidgetDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    const int m_avatarSize = 40;
    const int m_margin = 10;
    const int m_bubblePadding = 10;
};

#endif // CHAT_WIDGET_DELEGATE_H
