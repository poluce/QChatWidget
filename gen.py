import os

# 定义项目名称和目录
project_name = "WeChatListDemo"
if not os.path.exists(project_name):
    os.makedirs(project_name)

# ==========================================
# 1. ChatRoles.h (定义数据角色)
# ==========================================
code_roles_h = """#ifndef CHATROLES_H
#define CHATROLES_H

#include <Qt>

// 定义数据模型中的自定义角色
enum ChatRoles {
    NameRole = Qt::UserRole + 1, // 昵称
    MessageRole,                 // 最后一条消息
    TimeRole,                    // 时间
    AvatarColorRole,             // 头像颜色
    UnreadCountRole              // 未读消息数
};

#endif // CHATROLES_H
"""

# ==========================================
# 2. ChatDelegate.h (委托头文件)
# ==========================================
code_delegate_h = """#ifndef CHATDELEGATE_H
#define CHATDELEGATE_H

#include <QStyledItemDelegate>
#include "ChatRoles.h"

class ChatDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ChatDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // CHATDELEGATE_H
"""

# ==========================================
# 3. ChatDelegate.cpp (委托实现文件)
# ==========================================
code_delegate_cpp = """#include "ChatDelegate.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

ChatDelegate::ChatDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QSize ChatDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index);
    return QSize(option.rect.width(), 72); // 固定高度
}

void ChatDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // 1. 获取数据
    QString name = index.data(NameRole).toString();
    QString message = index.data(MessageRole).toString();
    QString time = index.data(TimeRole).toString();
    QColor avatarColor = index.data(AvatarColorRole).value<QColor>();
    int unreadCount = index.data(UnreadCountRole).toInt();

    // 2. 绘制背景
    QRect rect = option.rect;
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, QColor(195, 195, 195));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, QColor(220, 220, 220));
    } else {
        painter->fillRect(rect, Qt::white);
    }

    // 3. 布局参数
    int avatarSize = 50;
    int margin = 12;
    int textLeftMargin = margin + avatarSize + margin;

    // 4. 绘制头像
    QRect avatarRect(rect.left() + margin, rect.top() + (rect.height() - avatarSize) / 2, avatarSize, avatarSize);
    QPainterPath path;
    path.addEllipse(avatarRect);
    painter->setClipPath(path);
    painter->fillRect(avatarRect, avatarColor);
    painter->setClipping(false);

    // 5. 绘制未读红点
    if (unreadCount > 0) {
        int badgeSize = 18;
        QRect badgeRect(avatarRect.right() - 6, avatarRect.top() - 6, badgeSize, badgeSize);
        painter->setBrush(Qt::red);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badgeRect);
        
        painter->setPen(Qt::white);
        QFont badgeFont = painter->font();
        badgeFont.setPixelSize(10);
        painter->setFont(badgeFont);
        painter->drawText(badgeRect, Qt::AlignCenter, QString::number(unreadCount));
    }

    // 6. 绘制昵称
    painter->setPen(QColor(25, 25, 25));
    QFont nameFont = painter->font();
    nameFont.setPixelSize(16);
    painter->setFont(nameFont);
    
    QFont timeFont = painter->font();
    timeFont.setPixelSize(12);
    QFontMetrics fmTime(timeFont);
    int timeWidth = fmTime.horizontalAdvance(time) + margin;

    QRect nameRect(textLeftMargin, rect.top() + 14, rect.width() - textLeftMargin - timeWidth, 25);
    QString elidedName = painter->fontMetrics().elidedText(name, Qt::ElideRight, nameRect.width());
    painter->drawText(nameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedName);

    // 7. 绘制时间
    painter->setPen(QColor(180, 180, 180));
    painter->setFont(timeFont);
    QRect timeRect(rect.right() - timeWidth, rect.top() + 14, timeWidth - margin, 25);
    painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, time);

    // 8. 绘制消息内容
    painter->setPen(QColor(150, 150, 150));
    QFont msgFont = painter->font();
    msgFont.setPixelSize(14);
    painter->setFont(msgFont);
    
    QRect msgRect(textLeftMargin, rect.bottom() - 35, rect.width() - textLeftMargin - margin, 20);
    QString elidedMsg = painter->fontMetrics().elidedText(message, Qt::ElideRight, msgRect.width());
    painter->drawText(msgRect, Qt::AlignLeft | Qt::AlignVCenter, elidedMsg);

    // 9. 分隔线
    painter->setPen(QColor(230, 230, 230));
    painter->drawLine(textLeftMargin, rect.bottom(), rect.right(), rect.bottom());

    painter->restore();
}
"""

# ==========================================
# 4. ChatListView.h (列表视图头文件)
# ==========================================
code_listview_h = """#ifndef CHATLISTVIEW_H
#define CHATLISTVIEW_H

#include <QListView>

class ChatListView : public QListView {
    Q_OBJECT
public:
    explicit ChatListView(QWidget *parent = nullptr);
};

#endif // CHATLISTVIEW_H
"""

# ==========================================
# 5. ChatListView.cpp (列表视图实现)
# ==========================================
code_listview_cpp = """#include "ChatListView.h"
#include "ChatDelegate.h"

ChatListView::ChatListView(QWidget *parent) : QListView(parent) {
    this->setFrameShape(QFrame::NoFrame);
    this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->setUniformItemSizes(true);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // 设置自定义委托
    this->setItemDelegate(new ChatDelegate(this));
    
    this->setStyleSheet(
        "QListView { background-color: #FFFFFF; outline: none; }"
        "QListView::item { border-bottom: 1px solid transparent; }"
        "QListView::item:selected { background-color: #C3C3C3; }"
    );
}
"""

# ==========================================
# 6. main.cpp (主程序)
# ==========================================
code_main_cpp = """#include <QApplication>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStandardItemModel>
#include "ChatListView.h"
#include "ChatRoles.h"

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Qt WeChat List Demo (Modular)");
    window.resize(360, 600);

    QVBoxLayout *layout = new QVBoxLayout(&window);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QLineEdit *searchBar = new QLineEdit();
    searchBar->setPlaceholderText("搜索");
    searchBar->setStyleSheet("QLineEdit { border: 1px solid #ddd; border-radius: 4px; padding: 6px; margin: 10px; background: #f5f5f5; color: #333; }");
    layout->addWidget(searchBar);

    ChatListView *chatList = new ChatListView();
    QStandardItemModel *model = new QStandardItemModel(chatList);

    auto addChat = [&](const QString &name, const QString &msg, const QString &time, QColor color, int unread) {
        QStandardItem *item = new QStandardItem();
        item->setData(name, NameRole);
        item->setData(msg, MessageRole);
        item->setData(time, TimeRole);
        item->setData(color, AvatarColorRole);
        item->setData(unread, UnreadCountRole);
        model->appendRow(item);
    };

    addChat("文件传输助手", "[图片] IMG_2026.jpg", "17:52", QColor(255, 170, 0), 0);
    addChat("腾讯新闻", "Qt 6.8 发布了！", "14:30", QColor(0, 120, 215), 1);
    addChat("产品经理", "今晚加班吗？", "12:05", QColor(100, 100, 100), 5);
    
    for(int i = 0; i < 15; ++i) {
         addChat(QString("群聊 %1").arg(i), "收到请回复", "10:00", QColor(Qt::lightGray), 0);
    }

    chatList->setModel(model);
    layout->addWidget(chatList);

    window.show();
    return app.exec();
}
"""

# ==========================================
# 7. WeChatListDemo.pro (Qt 构建脚本/工程文件)
# ==========================================
code_pro = """QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WeChatListDemo
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \\
    main.cpp \\
    ChatDelegate.cpp \\
    ChatListView.cpp

HEADERS += \\
    ChatRoles.h \\
    ChatDelegate.h \\
    ChatListView.h
"""

# ==========================================
# 写入文件
# ==========================================
files = {
    "ChatRoles.h": code_roles_h,
    "ChatDelegate.h": code_delegate_h,
    "ChatDelegate.cpp": code_delegate_cpp,
    "ChatListView.h": code_listview_h,
    "ChatListView.cpp": code_listview_cpp,
    "main.cpp": code_main_cpp,
    "WeChatListDemo.pro": code_pro
}

print(f"开始生成项目: {project_name} ...")

for filename, content in files.items():
    file_path = os.path.join(project_name, filename)
    with open(file_path, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"  [+] 已生成: {filename}")

print("\\n项目生成成功！")
print(f"请使用 Qt Creator 打开文件夹 '{project_name}' 下的 'WeChatListDemo.pro' 文件即可运行。")