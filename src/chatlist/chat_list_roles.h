#ifndef CHAT_LIST_ROLES_H
#define CHAT_LIST_ROLES_H

#include <Qt>

// 定义数据模型中的自定义角色
enum ChatListRoles {
    NameRole = Qt::UserRole + 1, // 昵称
    MessageRole,                 // 最后一条消息
    TimeRole,                    // 时间
    AvatarColorRole,             // 头像颜色
    UnreadCountRole              // 未读消息数
};

#endif // CHAT_LIST_ROLES_H
