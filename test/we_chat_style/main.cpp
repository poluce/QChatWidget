#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "chatwidget/chat_widget.h"
#include "chatwidget/chat_widget_input.h"
#include "common/theme_manager.h"

class DemoInputWidget : public ChatWidgetInputBase {
public:
    explicit DemoInputWidget(QWidget* parent = nullptr)
        : ChatWidgetInputBase(parent)
    {
        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setSpacing(6);

        m_edit = new QLineEdit(this);
        m_edit->setPlaceholderText("简易输入...");
        auto* sendButton = new QPushButton("简易发送", this);
        auto* stopButton = new QPushButton("停止", this);

        layout->addWidget(m_edit, 1);
        layout->addWidget(sendButton);
        layout->addWidget(stopButton);

        auto send = [this]() {
            const QString text = m_edit->text().trimmed();
            if (text.isEmpty()) {
                return;
            }
            emit messageSent(text);
            m_edit->clear();
        };
        QObject::connect(sendButton, &QPushButton::clicked, this, send);
        QObject::connect(m_edit, &QLineEdit::returnPressed, this, send);
        QObject::connect(stopButton, &QPushButton::clicked, this, [this]() {
            emit stopRequested();
        });
    }

private:
    QLineEdit* m_edit = nullptr;
};

int main(int argc, char* argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("ChatWidget 控制面板演示");
    window.resize(1100, 720);

    auto* rootLayout = new QHBoxLayout(&window);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(10);

    auto* chat = new ChatWidget(&window);
    chat->setObjectName("demoChatWidget");
    chat->applyStyleSheetFile("chat_widget.qss");
    rootLayout->addWidget(chat, 3);

    auto* panelRoot = new QWidget(&window);
    auto* panelLayout = new QVBoxLayout(panelRoot);
    panelLayout->setContentsMargins(8, 8, 8, 8);
    panelLayout->setSpacing(10);

    auto* statusLabel = new QLabel("Ready", panelRoot);
    statusLabel->setWordWrap(true);
    statusLabel->setObjectName("demoStatusLabel");
    panelLayout->addWidget(statusLabel);

    auto* scrollArea = new QScrollArea(&window);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setMinimumWidth(320);
    scrollArea->setWidget(panelRoot);
    rootLayout->addWidget(scrollArea, 1);

    auto setStatus = [statusLabel](const QString& text) {
        statusLabel->setText(text);
    };

    auto ensureDefaultInput = [chat, setStatus]() -> bool {
        if (!qobject_cast<ChatWidgetInput*>(chat->inputWidget())) {
            setStatus("当前为自定义输入组件，请先恢复默认输入。");
            return false;
        }
        return true;
    };

    auto makeSection = [panelRoot, panelLayout](const QString& title) {
        auto* box = new QGroupBox(title, panelRoot);
        auto* layout = new QVBoxLayout(box);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(6);
        panelLayout->addWidget(box);
        return layout;
    };

    auto addButton = [](QVBoxLayout* layout, const QString& text) {
        auto* btn = new QPushButton(text);
        layout->addWidget(btn);
        return btn;
    };

    auto applyTheme = [chat]() {
        auto themeStyle = ThemeManager::instance()->chatWidgetStyle();
        ChatWidgetDelegate::Style style;
        style.avatarSize = themeStyle.avatarSize;
        style.margin = themeStyle.margin;
        style.bubblePadding = themeStyle.bubblePadding;
        style.bubbleRadius = themeStyle.bubbleRadius;
        style.myBubbleColor = themeStyle.myBubbleColor;
        style.otherBubbleColor = themeStyle.otherBubbleColor;
        style.myAvatarColor = themeStyle.myAvatarColor;
        style.otherAvatarColor = themeStyle.otherAvatarColor;
        style.myTextColor = themeStyle.myTextColor;
        style.otherTextColor = themeStyle.otherTextColor;
        style.messageFont = themeStyle.messageFont;
        style.avatarFont = themeStyle.avatarFont;
        chat->setDelegateStyle(style);
    };

    auto bindDefaultInputSignals = [chat, setStatus]() {
        auto* input = qobject_cast<ChatWidgetInput*>(chat->inputWidget());
        if (!input) {
            return;
        }
        QObject::connect(input, &ChatWidgetInput::voiceStartRequested, input, [setStatus]() {
            setStatus("语音开始（voiceStartRequested）");
        });
        QObject::connect(input, &ChatWidgetInput::voiceStopRequested, input, [setStatus]() {
            setStatus("语音停止（voiceStopRequested）");
        });
        QObject::connect(input, &ChatWidgetInput::emojiSelected, input, [setStatus](const QString& emoji) {
            setStatus(QString("emojiSelected: %1").arg(emoji));
        });
        QObject::connect(input, &ChatWidgetInput::richTextToggled, input, [setStatus](bool enabled) {
            setStatus(QString("richTextToggled: %1").arg(enabled ? "ON" : "OFF"));
        });
        QObject::connect(input, &ChatWidgetInput::draftChanged, input, [setStatus](const QString& text) {
            setStatus(QString("draftChanged: %1").arg(text.left(24)));
        });
    };

    applyTheme();
    bindDefaultInputSignals();

    {
        ChatWidget::MessageParams params;
        params.content = "**Hello!** 欢迎使用 ChatWidget 控制面板演示。";
        params.displayName = "AI";
        params.isMine = false;
        chat->addMessage(params);
    }

    QObject::connect(chat, &ChatWidget::messageSent, chat, [&](const QString& content) {
        setStatus(QString("messageSent: %1").arg(content));
        QTimer::singleShot(800, chat, [chat, content]() {
            chat->startSimulatedStreaming(QString("AI 回复: %1").arg(content), 30);
        });
    });

    QObject::connect(chat, &ChatWidget::stopRequested, chat, [setStatus]() {
        setStatus("stopRequested: 用户点击停止");
        qDebug() << "User requested stop. Timer handled internally by setSendingState(false).";
    });

    QObject::connect(chat, &ChatWidget::avatarClicked, chat, [setStatus](const QString& name, bool isMine, int row) {
        setStatus(QString("头像点击: %1 (isMine=%2, row=%3)").arg(name).arg(isMine).arg(row));
    });
    QObject::connect(chat, &ChatWidget::selfAvatarClicked, chat, [setStatus](const QString& id, int row) {
        setStatus(QString("点击自己头像: %1 row=%2").arg(id).arg(row));
    });
    QObject::connect(chat, &ChatWidget::memberAvatarClicked, chat, [setStatus](const QString& id, const QString& name, int row) {
        setStatus(QString("点击成员头像: %1(%2) row=%3").arg(name, id).arg(row));
    });
    QObject::connect(chat, &ChatWidget::messageSelected, chat, [setStatus](const QString& messageId) {
        setStatus(QString("messageSelected: %1").arg(messageId));
    });
    QObject::connect(chat, &ChatWidget::messageContextMenuRequested, chat, [setStatus](const QString& messageId, const QPoint& pos) {
        setStatus(QString("messageContextMenuRequested: %1 @%2,%3").arg(messageId).arg(pos.x()).arg(pos.y()));
    });
    QObject::connect(chat, &ChatWidget::messageActionRequested, chat, [setStatus](const QString& action, const QString& messageId) {
        setStatus(QString("messageActionRequested: %1 on %2").arg(action, messageId));
    });

    bool hasHistory = false;

    auto makeBaseHistory = []() {
        QList<ChatWidget::HistoryMessage> history;
        const QDateTime now = QDateTime::currentDateTime();

        ChatWidget::HistoryMessage date;
        date.content = "昨天";
        date.timestamp = now.addDays(-1);
        date.messageId = "d1";
        date.messageType = ChatWidgetMessage::MessageType::DateSeparator;
        history.append(date);

        ChatWidget::HistoryMessage sys;
        sys.content = "系统提示：已开启主流 UI 渲染";
        sys.timestamp = now.addSecs(-320);
        sys.messageId = "sys1";
        sys.messageType = ChatWidgetMessage::MessageType::System;
        history.append(sys);

        ChatWidget::HistoryMessage a;
        a.senderId = "u_bob";
        a.displayName = "Bob";
        a.content = "历史消息 1";
        a.timestamp = now.addSecs(-300);
        a.messageId = "h1";
        history.append(a);

        ChatWidget::HistoryMessage b;
        b.senderId = "u_me";
        b.displayName = "Me";
        b.content = "历史消息 2 @Alice";
        b.timestamp = now.addSecs(-200);
        b.messageId = "h2";
        b.status = ChatWidgetMessage::MessageStatus::Read;
        b.mentions = QStringList() << "Alice";
        b.reactions.append(ChatWidgetReaction{ "👍", 2 });
        history.append(b);

        ChatWidget::HistoryMessage c;
        c.senderId = "u_alice";
        c.displayName = "Alice";
        c.content = "历史消息 3";
        c.timestamp = now.addSecs(-100);
        c.messageId = "h3";
        c.replyToMessageId = "h1";
        c.replySender = "Bob";
        c.replyPreview = "历史消息 1";
        history.append(c);

        ChatWidget::HistoryMessage fileMsg;
        fileMsg.senderId = "u_bob";
        fileMsg.displayName = "Bob";
        fileMsg.content = "发送了一个文件";
        fileMsg.timestamp = now.addSecs(-80);
        fileMsg.messageId = "h4";
        fileMsg.messageType = ChatWidgetMessage::MessageType::File;
        fileMsg.fileName = "报价单.pdf";
        fileMsg.fileSize = 234567;
        history.append(fileMsg);

        ChatWidget::HistoryMessage imageMsg;
        imageMsg.senderId = "u_alice";
        imageMsg.displayName = "Alice";
        imageMsg.content = "图片预览";
        imageMsg.timestamp = now.addSecs(-60);
        imageMsg.messageId = "h5";
        imageMsg.messageType = ChatWidgetMessage::MessageType::Image;
        imageMsg.imagePath = "missing.png";
        history.append(imageMsg);

        return history;
    };

    auto makeAppendHistory = []() {
        QList<ChatWidget::HistoryMessage> history;
        const QDateTime now = QDateTime::currentDateTime();

        ChatWidget::HistoryMessage dup;
        dup.senderId = "u_bob";
        dup.displayName = "Bob";
        dup.content = "历史消息 3（重复）";
        dup.timestamp = now.addSecs(-90);
        dup.messageId = "h3";
        history.append(dup);

        ChatWidget::HistoryMessage d;
        d.senderId = "u_bob";
        d.displayName = "Bob";
        d.content = "追加历史消息 4";
        d.timestamp = now.addSecs(-60);
        d.messageId = "h4";
        history.append(d);

        ChatWidget::HistoryMessage e;
        e.senderId = "u_me";
        e.displayName = "Me";
        e.content = "追加历史消息 5";
        e.timestamp = now.addSecs(-30);
        e.messageId = "h5";
        history.append(e);

        return history;
    };

    auto makePrependHistory = []() {
        QList<ChatWidget::HistoryMessage> history;
        const QDateTime now = QDateTime::currentDateTime();

        ChatWidget::HistoryMessage z;
        z.senderId = "u_alice";
        z.displayName = "Alice";
        z.content = "更早的历史消息 0";
        z.timestamp = now.addSecs(-600);
        z.messageId = "h0";
        history.append(z);

        ChatWidget::HistoryMessage dup;
        dup.senderId = "u_bob";
        dup.displayName = "Bob";
        dup.content = "历史消息 1（重复）";
        dup.timestamp = now.addSecs(-500);
        dup.messageId = "h1";
        history.append(dup);

        return history;
    };

    auto* msgSection = makeSection("基础消息");
    auto* addMine = addButton(msgSection, "添加我的消息（bool）");
    QObject::connect(addMine, &QPushButton::clicked, chat, [chat, setStatus]() {
        ChatWidget::MessageParams params;
        params.content = "这是一条我的消息（bool 重载）";
        params.displayName = "Me";
        params.isMine = true;
        chat->addMessage(params);
        setStatus("添加我的消息（bool 重载）");
    });
    auto* addById = addButton(msgSection, "添加他人消息（senderId）");
    QObject::connect(addById, &QPushButton::clicked, chat, [chat, setStatus]() {
        ChatWidget::MessageParams params;
        params.content = "你好，我是 Bob（senderId 重载）";
        params.senderId = "u_bob";
        chat->addMessage(params);
        setStatus("添加他人消息（senderId 重载）");
    });
    auto* addByFull = addButton(msgSection, "添加他人消息（完整信息）");
    QObject::connect(addByFull, &QPushButton::clicked, chat, [chat, setStatus]() {
        ChatWidget::MessageParams params;
        params.content = "我是 Alice（完整信息）";
        params.senderId = "u_alice";
        params.displayName = "Alice";
        chat->addMessage(params);
        setStatus("添加他人消息（完整信息）");
    });
    auto* removeLast = addButton(msgSection, "移除最后一条");
    QObject::connect(removeLast, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->removeLastMessage();
        setStatus("移除最后一条消息");
    });
    auto* clearAll = addButton(msgSection, "清空消息");
    QObject::connect(clearAll, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->clearMessages();
        setStatus("清空所有消息");
    });
    auto* countBtn = addButton(msgSection, "消息计数");
    QObject::connect(countBtn, &QPushButton::clicked, chat, [chat, setStatus]() {
        setStatus(QString("当前消息数: %1").arg(chat->messageCount()));
    });

    auto* streamSection = makeSection("流式与发送状态");
    auto* startStream = addButton(streamSection, "开始模拟流式");
    QObject::connect(startStream, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->startSimulatedStreaming("模拟 AI 流式输出：你好，世界！", 30);
        setStatus("开始模拟流式输出");
    });
    auto* appendStream = addButton(streamSection, "追加流式片段");
    QObject::connect(appendStream, &QPushButton::clicked, chat, [chat, setStatus]() {
        ChatWidget::MessageParams params;
        params.content = QString();
        params.displayName = "AI";
        params.isMine = false;
        chat->addMessage(params);
        chat->setSendingState(true);
        chat->streamOutput("这是一段手动追加的流式内容。");
        setStatus("手动追加流式片段");
    });
    auto* setSending = addButton(streamSection, "置为发送中");
    QObject::connect(setSending, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setSendingState(true);
        setStatus("setSendingState(true)");
    });
    auto* stopSending = addButton(streamSection, "停止发送/流式");
    QObject::connect(stopSending, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setSendingState(false);
        setStatus("setSendingState(false)");
    });

    auto* participantSection = makeSection("参与者管理");
    auto* setCurrentUser = addButton(participantSection, "设置当前用户");
    QObject::connect(setCurrentUser, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setCurrentUser("u_me", "Me");
        setStatus("设置当前用户: u_me");
    });
    auto* upsertAlice = addButton(participantSection, "Upsert 参与者 Alice");
    QObject::connect(upsertAlice, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->upsertParticipant("u_alice", "Alice");
        ChatWidget::MessageParams params;
        params.content = "Alice 刚加入";
        params.senderId = "u_alice";
        params.displayName = "Alice";
        chat->addMessage(params);
        setStatus("upsertParticipant: u_alice");
    });
    auto* updateAlice = addButton(participantSection, "更新 Alice 信息");
    QObject::connect(updateAlice, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->upsertParticipant("u_alice", "Alice ✨");
        setStatus("upsertParticipant: u_alice -> Alice ✨");
    });
    auto* removeAlice = addButton(participantSection, "移除 Alice");
    QObject::connect(removeAlice, &QPushButton::clicked, chat, [chat, setStatus]() {
        const bool removed = chat->removeParticipant("u_alice");
        setStatus(QString("removeParticipant(u_alice): %1").arg(removed));
    });
    auto* clearParticipants = addButton(participantSection, "清空参与者");
    QObject::connect(clearParticipants, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->clearParticipants();
        setStatus("clearParticipants()");
    });
    auto* hasBob = addButton(participantSection, "检查 Bob 是否存在");
    QObject::connect(hasBob, &QPushButton::clicked, chat, [chat, setStatus]() {
        const bool exists = chat->hasParticipant("u_bob");
        setStatus(QString("hasParticipant(u_bob): %1").arg(exists));
    });

    auto* historySection = makeSection("历史消息");
    auto* setHistory = addButton(historySection, "载入历史（重置）");
    QObject::connect(setHistory, &QPushButton::clicked, chat, [chat, setStatus, &hasHistory, makeBaseHistory]() {
        chat->setHistoryMessages(makeBaseHistory(), true);
        hasHistory = true;
        setStatus("setHistoryMessages: 重置并加载历史");
    });
    auto* appendHistory = addButton(historySection, "追加历史（去重）");
    QObject::connect(appendHistory, &QPushButton::clicked, chat, [chat, setStatus, &hasHistory, makeAppendHistory]() {
        if (!hasHistory) {
            setStatus("请先点击 \"载入历史（重置）\"");
            return;
        }
        chat->appendHistoryMessages(makeAppendHistory(), true);
        setStatus("appendHistoryMessages: 已追加（含重复 ID）");
    });
    auto* prependHistory = addButton(historySection, "前插历史（去重）");
    QObject::connect(prependHistory, &QPushButton::clicked, chat, [chat, setStatus, &hasHistory, makePrependHistory]() {
        if (!hasHistory) {
            setStatus("请先点击 \"载入历史（重置）\"");
            return;
        }
        chat->prependHistoryMessages(makePrependHistory(), true);
        setStatus("prependHistoryMessages: 已前插（含重复 ID）");
    });

    auto* emptySection = makeSection("空状态");
    auto* showEmpty = addButton(emptySection, "显示空状态");
    QObject::connect(showEmpty, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setEmptyStateVisible(true, "空状态");
        setStatus("setEmptyStateVisible(true)");
    });
    auto* hideEmpty = addButton(emptySection, "隐藏空状态");
    QObject::connect(hideEmpty, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setEmptyStateVisible(false);
        setStatus("setEmptyStateVisible(false)");
    });
    auto* checkEmpty = addButton(emptySection, "查询空状态");
    QObject::connect(checkEmpty, &QPushButton::clicked, chat, [chat, setStatus]() {
        setStatus(QString("isEmptyStateVisible: %1").arg(chat->isEmptyStateVisible()));
    });

    auto* inputSection = makeSection("输入与交互");
    auto* showCommand = addButton(inputSection, "触发命令菜单");
    QObject::connect(showCommand, &QPushButton::clicked, chat, [chat, setStatus, ensureDefaultInput]() {
        if (!ensureDefaultInput()) {
            return;
        }
        auto* edit = chat->findChild<QLineEdit*>("chatWidgetInputEdit");
        if (!edit) {
            setStatus("未找到输入框");
            return;
        }
        edit->setText("/t");
        edit->setFocus();
        setStatus("已设置 /t 触发命令菜单");
    });
    auto* switchTrans = addButton(inputSection, "切换翻译模式");
    QObject::connect(switchTrans, &QPushButton::clicked, chat, [chat, setStatus, ensureDefaultInput]() {
        if (!ensureDefaultInput()) {
            return;
        }
        auto* edit = chat->findChild<QLineEdit*>("chatWidgetInputEdit");
        auto* send = chat->findChild<QPushButton*>("chatWidgetInputSendButton");
        if (!edit || !send) {
            setStatus("未找到输入框或发送按钮");
            return;
        }
        edit->setText("/trans");
        send->click();
        setStatus("已发送 /trans 命令");
    });
    auto* toggleVoice = addButton(inputSection, "模拟语音按钮");
    QObject::connect(toggleVoice, &QPushButton::clicked, chat, [chat, setStatus, ensureDefaultInput]() {
        if (!ensureDefaultInput()) {
            return;
        }
        auto* voice = chat->findChild<QPushButton*>("chatWidgetInputVoiceButton");
        if (!voice) {
            setStatus("未找到语音按钮");
            return;
        }
        voice->click();
        setStatus("已触发语音按钮 click");
    });
    auto* openPlus = addButton(inputSection, "打开附件菜单");
    QObject::connect(openPlus, &QPushButton::clicked, chat, [chat, setStatus, ensureDefaultInput]() {
        if (!ensureDefaultInput()) {
            return;
        }
        auto* plus = chat->findChild<QToolButton*>("chatWidgetInputPlusButton");
        if (!plus) {
            setStatus("未找到 + 按钮");
            return;
        }
        plus->showMenu();
        setStatus("已打开附件菜单");
    });
    auto* toggleRich = addButton(inputSection, "切换富文本按钮");
    QObject::connect(toggleRich, &QPushButton::clicked, chat, [chat, setStatus, ensureDefaultInput]() {
        if (!ensureDefaultInput()) {
            return;
        }
        auto* rich = chat->findChild<QToolButton*>("chatWidgetInputRichButton");
        if (!rich) {
            setStatus("未找到富文本按钮");
            return;
        }
        rich->click();
        setStatus("已切换富文本按钮");
    });
    auto* openEmoji = addButton(inputSection, "打开表情菜单");
    QObject::connect(openEmoji, &QPushButton::clicked, chat, [chat, setStatus, ensureDefaultInput]() {
        if (!ensureDefaultInput()) {
            return;
        }
        auto* emoji = chat->findChild<QToolButton*>("chatWidgetInputEmojiButton");
        if (!emoji) {
            setStatus("未找到表情按钮");
            return;
        }
        emoji->showMenu();
        setStatus("已打开表情菜单");
    });

    auto* inputWidgetSection = makeSection("输入组件");
    auto* useSimpleInput = addButton(inputWidgetSection, "切换为简易输入");
    QObject::connect(useSimpleInput, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setInputWidget(new DemoInputWidget());
        setStatus("已切换为简易输入组件");
    });
    auto* restoreDefaultInput = addButton(inputWidgetSection, "恢复默认输入");
    QObject::connect(restoreDefaultInput, &QPushButton::clicked, chat, [chat, setStatus, bindDefaultInputSignals]() {
        chat->setInputWidget(new ChatWidgetInput());
        bindDefaultInputSignals();
        setStatus("已恢复默认输入组件");
    });

    auto* styleSection = makeSection("主题与样式");
    auto* toggleTheme = addButton(styleSection, "切换主题 Light/Dark");
    QObject::connect(toggleTheme, &QPushButton::clicked, chat, [setStatus, applyTheme]() {
        auto* manager = ThemeManager::instance();
        const auto next = manager->currentTheme() == ThemeManager::Light ? ThemeManager::Dark : ThemeManager::Light;
        manager->setTheme(next);
        applyTheme();
        setStatus(QString("主题切换为: %1").arg(next == ThemeManager::Light ? "Light" : "Dark"));
    });
    auto* applyThemeBtn = addButton(styleSection, "应用主题样式");
    QObject::connect(applyThemeBtn, &QPushButton::clicked, chat, [applyTheme, setStatus]() {
        applyTheme();
        setStatus("已应用 ThemeManager 样式");
    });
    auto* tweakStyle = addButton(styleSection, "自定义气泡圆角");
    QObject::connect(tweakStyle, &QPushButton::clicked, chat, [chat, setStatus]() {
        auto style = chat->delegateStyle();
        style.bubbleRadius = style.bubbleRadius + 6;
        chat->setDelegateStyle(style);
        setStatus("已增加气泡圆角");
    });
    auto* reloadQss = addButton(styleSection, "重新加载 QSS");
    QObject::connect(reloadQss, &QPushButton::clicked, chat, [chat, setStatus]() {
        const bool ok = chat->applyStyleSheetFile("chat_widget.qss");
        setStatus(QString("重新加载 QSS: %1").arg(ok));
    });

    auto* metaSection = makeSection("消息元数据");
    auto* setSearch = addButton(metaSection, "设置搜索关键字: Alice");
    QObject::connect(setSearch, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setSearchKeyword("Alice");
        setStatus("setSearchKeyword(Alice)");
    });
    auto* clearSearch = addButton(metaSection, "清除搜索关键字");
    QObject::connect(clearSearch, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->setSearchKeyword(QString());
        setStatus("clear search keyword");
    });
    auto* updateStatus = addButton(metaSection, "更新 h2 状态=已读");
    QObject::connect(updateStatus, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->updateMessageStatus("h2", ChatWidgetMessage::MessageStatus::Read);
        setStatus("updateMessageStatus(h2=Read)");
    });
    auto* updateReactions = addButton(metaSection, "更新 h4 Reactions");
    QObject::connect(updateReactions, &QPushButton::clicked, chat, [chat, setStatus]() {
        QList<ChatWidgetReaction> reactions;
        reactions.append(ChatWidgetReaction{ "🔥", 1 });
        reactions.append(ChatWidgetReaction{ "👍", 3 });
        chat->updateMessageReactions("h4", reactions);
        setStatus("updateMessageReactions(h4)");
    });
    auto* updateReply = addButton(metaSection, "更新 h4 回复预览");
    QObject::connect(updateReply, &QPushButton::clicked, chat, [chat, setStatus]() {
        chat->updateMessageReply("h4", "h1", "Bob", "历史消息 1", false, QString());
        setStatus("updateMessageReply(h4)");
    });

    panelLayout->addStretch(1);

    window.show();
    return app.exec();
}
