#include "chat_list_widget.h"
#include "chat_list_view.h"
#include "chat_list_filter_model.h"
#include "chat_list_roles.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QItemSelectionModel>

ChatListWidget::ChatListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

ChatListWidget::~ChatListWidget()
{
}

void ChatListWidget::setupUi()
{
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText("搜索");
    m_searchBar->setStyleSheet(
        "QLineEdit { border: 1px solid #ddd; border-radius: 6px; padding: 6px 10px; margin: 10px; background: #f5f5f5; color: #333; }"
    );

    m_listView = new ChatListView(this);
    m_filterModel = new ChatListFilterModel(this);
    m_filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    applyDefaultStyle();

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->addWidget(m_searchBar);
    m_layout->addWidget(m_listView);

    connect(m_searchBar, &QLineEdit::textChanged, this, &ChatListWidget::searchTextChanged);
    connect(m_searchBar, &QLineEdit::textChanged, this, &ChatListWidget::applyFilterText);
    connect(m_listView, &ChatListView::chatItemActivated, this, &ChatListWidget::chatItemActivated);
    connect(m_listView, &QListView::clicked, this, &ChatListWidget::clicked);
    connect(m_listView, &QListView::doubleClicked, this, &ChatListWidget::doubleClicked);
    connect(m_listView, &QListView::pressed, this, &ChatListWidget::pressed);
    connect(m_listView, &QListView::activated, this, &ChatListWidget::activated);
    connect(m_listView, &QListView::entered, this, &ChatListWidget::entered);
    connect(m_listView, &QListView::viewportEntered, this, &ChatListWidget::viewportEntered);
    wireSelectionSignals();
}

ChatListView *ChatListWidget::listView() const
{
    return m_listView;
}

QLineEdit *ChatListWidget::searchBar() const
{
    return m_searchBar;
}

void ChatListWidget::applyDefaultStyle()
{
    m_listView->setItemHeight(76);
    m_listView->setAvatarSize(52);
    m_listView->setShowSeparator(true);
    m_listView->setHoverColor(QColor(236, 238, 242));
    m_listView->setSelectedColor(QColor(220, 224, 230));
}

void ChatListWidget::setSearchPlaceholder(const QString &text)
{
    m_searchBar->setPlaceholderText(text);
}

void ChatListWidget::setSearchVisible(bool visible)
{
    m_searchBar->setVisible(visible);
}

void ChatListWidget::setSearchStyleSheet(const QString &style)
{
    m_searchBar->setStyleSheet(style);
}

void ChatListWidget::enableSearchFiltering(bool enabled)
{
    m_filterEnabled = enabled;
    if (m_filterEnabled) {
        QAbstractItemModel *source = m_listView->standardModel();
        m_filterModel->setSourceModel(source);
        m_listView->setModel(m_filterModel);
        if (m_filterModel->searchRoles().isEmpty()) {
            m_filterModel->setSearchRoles(QList<int>() << NameRole << MessageRole);
        }
        applyFilterText(m_searchBar->text());
    } else {
        if (m_filterModel->sourceModel()) {
            m_listView->setModel(m_filterModel->sourceModel());
        }
    }
    wireSelectionSignals();
}

void ChatListWidget::setSearchRoles(const QList<int> &roles)
{
    m_filterModel->setSearchRoles(roles);
}

void ChatListWidget::setSearchCaseSensitivity(Qt::CaseSensitivity sensitivity)
{
    m_filterModel->setFilterCaseSensitivity(sensitivity);
}

int ChatListWidget::addChatItem(const QString &name,
                                const QString &message,
                                const QString &time,
                                const QColor &avatarColor,
                                int unreadCount)
{
    return m_listView->addChatItem(name, message, time, avatarColor, unreadCount);
}

void ChatListWidget::updateChatItem(int row,
                                    const QString &name,
                                    const QString &message,
                                    const QString &time,
                                    const QColor &avatarColor,
                                    int unreadCount)
{
    m_listView->updateChatItem(row, name, message, time, avatarColor, unreadCount);
}

bool ChatListWidget::updateChatItemData(int row, int role, const QVariant &value)
{
    return m_listView->updateChatItemData(row, role, value);
}

bool ChatListWidget::updateChatItemData(int row, const QHash<int, QVariant> &values)
{
    return m_listView->updateChatItemData(row, values);
}

int ChatListWidget::findRowByName(const QString &name) const
{
    return m_listView->findRowByName(name);
}

QList<int> ChatListWidget::findRowsByName(const QString &name) const
{
    return m_listView->findRowsByName(name);
}

bool ChatListWidget::updateChatItemByName(const QString &name, int role, const QVariant &value)
{
    return m_listView->updateChatItemByName(name, role, value);
}

bool ChatListWidget::updateChatItemByName(const QString &name, const QHash<int, QVariant> &values)
{
    return m_listView->updateChatItemByName(name, values);
}

int ChatListWidget::updateChatItemsByName(const QString &name, int role, const QVariant &value)
{
    return m_listView->updateChatItemsByName(name, role, value);
}

int ChatListWidget::updateChatItemsByName(const QString &name, const QHash<int, QVariant> &values)
{
    return m_listView->updateChatItemsByName(name, values);
}

bool ChatListWidget::removeChatItem(int row)
{
    return m_listView->removeChatItem(row);
}

bool ChatListWidget::removeChatItemByName(const QString &name)
{
    return m_listView->removeChatItemByName(name);
}

int ChatListWidget::removeChatItemsByName(const QString &name)
{
    return m_listView->removeChatItemsByName(name);
}

void ChatListWidget::clearChats()
{
    m_listView->clearChats();
}

void ChatListWidget::applyFilterText(const QString &text)
{
    if (!m_filterEnabled) {
        return;
    }
    QRegularExpression re(QRegularExpression::escape(text), QRegularExpression::CaseInsensitiveOption);
    if (m_filterModel->filterCaseSensitivity() == Qt::CaseSensitive) {
        re = QRegularExpression(QRegularExpression::escape(text));
    }
    m_filterModel->setFilterRegularExpression(re);
}

void ChatListWidget::wireSelectionSignals()
{
    QItemSelectionModel *current = m_listView->selectionModel();
    if (current == m_selectionModel) {
        return;
    }
    if (m_selectionModel) {
        disconnect(m_selectionModel, nullptr, this, nullptr);
    }
    m_selectionModel = current;
    if (!m_selectionModel) {
        return;
    }
    connect(m_selectionModel, &QItemSelectionModel::currentChanged, this, &ChatListWidget::currentChanged);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &ChatListWidget::selectionChanged);
}
