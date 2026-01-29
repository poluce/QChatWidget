#include "chat_list_filter_model.h"
#include <QAbstractItemModel>
#include <QRegularExpression>

ChatListFilterModel::ChatListFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

void ChatListFilterModel::setSearchRoles(const QList<int>& roles)
{
    m_roles = roles;
    invalidateFilter();
}

QList<int> ChatListFilterModel::searchRoles() const
{
    return m_roles;
}

bool ChatListFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const QRegularExpression re = filterRegularExpression();
    if (!re.isValid() || re.pattern().isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    const QList<int> roles = m_roles.isEmpty() ? QList<int>() : m_roles;

    if (roles.isEmpty()) {
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }

    for (int role : roles) {
        const QString text = index.data(role).toString();
        if (re.match(text).hasMatch()) {
            return true;
        }
    }
    return false;
}
