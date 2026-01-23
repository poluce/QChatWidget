#ifndef CHATLISTFILTERMODEL_H
#define CHATLISTFILTERMODEL_H

#include <QSortFilterProxyModel>

class ChatListFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ChatListFilterModel(QObject *parent = nullptr);

    void setSearchRoles(const QList<int> &roles);
    QList<int> searchRoles() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QList<int> m_roles;
};

#endif // CHATLISTFILTERMODEL_H
