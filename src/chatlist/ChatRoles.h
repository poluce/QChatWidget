#ifndef CHATROLES_H
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
