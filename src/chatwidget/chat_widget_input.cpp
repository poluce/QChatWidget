#include "chat_widget_input.h"
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolButton>
#include <QVBoxLayout>

ChatWidgetInput::ChatWidgetInput(QWidget* parent)
    : ChatWidgetInputBase(parent)
{
    setupUi();
}

ChatWidgetInput::~ChatWidgetInput()
{
}

void ChatWidgetInput::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_commandMenu->isVisible()) {
        positionCommandMenu();
    }
}

void ChatWidgetInput::setupUi()
{
    setObjectName("chatWidgetInputRoot");

    m_inputBar = new QFrame(this);
    m_inputBar->setObjectName("chatWidgetInputBar");

    m_plusButton = new QToolButton(m_inputBar);
    m_plusButton->setObjectName("chatWidgetInputPlusButton");
    m_plusButton->setText("+");
    m_plusButton->setProperty("role", "icon");
    m_plusButton->setPopupMode(QToolButton::InstantPopup);

    m_emojiButton = new QToolButton(m_inputBar);
    m_emojiButton->setObjectName("chatWidgetInputEmojiButton");
    m_emojiButton->setText("😊");
    m_emojiButton->setProperty("role", "icon");
    m_emojiButton->setPopupMode(QToolButton::InstantPopup);

    m_voiceButton = new QPushButton("语音", m_inputBar);
    m_voiceButton->setObjectName("chatWidgetInputVoiceButton");
    m_voiceButton->setProperty("role", "icon");
    m_inputEdit = new QLineEdit(m_inputBar);
    m_inputEdit->setObjectName("chatWidgetInputEdit");
    m_inputEdit->setPlaceholderText("输入消息...");
    m_richTextButton = new QToolButton(m_inputBar);
    m_richTextButton->setObjectName("chatWidgetInputRichButton");
    m_richTextButton->setText("A");
    m_richTextButton->setCheckable(true);
    m_richTextButton->setProperty("role", "icon");
    m_sendButton = new QPushButton("发送", m_inputBar);
    m_sendButton->setObjectName("chatWidgetInputSendButton");
    m_sendButton->setProperty("role", "primary");

    m_plusMenu = new QMenu(this);
    m_plusMenu->setObjectName("chatWidgetInputMenu");
    m_pickImageAction = m_plusMenu->addAction("图片");
    m_pickFileAction = m_plusMenu->addAction("文件");
    m_plusButton->setMenu(m_plusMenu);

    m_emojiMenu = new QMenu(this);
    m_emojiMenu->setObjectName("chatWidgetInputEmojiMenu");
    const QStringList emojis = { "😀", "😂", "😍", "👍", "🎉", "🔥", "🙏", "✅", "✨", "😅" };
    for (const QString& emoji : emojis) {
        QAction* action = m_emojiMenu->addAction(emoji);
        action->setData(emoji);
    }
    m_emojiButton->setMenu(m_emojiMenu);

    m_commandMenu = new QListWidget(this);
    m_commandMenu->setObjectName("chatWidgetCommandMenu");
    m_commandMenu->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_commandMenu->setFocusPolicy(Qt::NoFocus);
    m_commandMenu->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_commandMenu->setSelectionMode(QAbstractItemView::SingleSelection);
    m_commandMenu->setSpacing(2);

    m_shortcutLabel = new QLabel("Enter 发送 · Ctrl+Enter 换行", this);
    m_shortcutLabel->setObjectName("chatWidgetInputShortcutLabel");

    QGraphicsDropShadowEffect* barShadow = new QGraphicsDropShadowEffect(this);
    barShadow->setBlurRadius(18);
    barShadow->setOffset(0, 3);
    barShadow->setColor(QColor(0, 0, 0, 45));
    m_inputBar->setGraphicsEffect(barShadow);

    QGraphicsDropShadowEffect* menuShadow = new QGraphicsDropShadowEffect(this);
    menuShadow->setBlurRadius(18);
    menuShadow->setOffset(0, 4);
    menuShadow->setColor(QColor(0, 0, 0, 45));
    m_commandMenu->setGraphicsEffect(menuShadow);

    QHBoxLayout* inputLayout = new QHBoxLayout(m_inputBar);
    inputLayout->setContentsMargins(10, 6, 10, 6);
    inputLayout->setSpacing(8);
    inputLayout->addWidget(m_plusButton);
    inputLayout->addWidget(m_emojiButton);
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_richTextButton);
    inputLayout->addWidget(m_voiceButton);
    inputLayout->addWidget(m_sendButton);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 8, 10, 12);
    rootLayout->addWidget(m_inputBar);
    rootLayout->addWidget(m_shortcutLabel, 0, Qt::AlignRight);

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidgetInput::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ChatWidgetInput::onSendClicked);
    connect(m_inputEdit, &QLineEdit::textChanged, this, &ChatWidgetInput::onInputTextChanged);
    connect(m_commandMenu, &QListWidget::itemClicked, this, &ChatWidgetInput::onCommandClicked);
    connect(m_voiceButton, &QPushButton::clicked, this, &ChatWidgetInput::onVoiceClicked);
    connect(m_pickImageAction, &QAction::triggered, this, &ChatWidgetInput::onPickImage);
    connect(m_pickFileAction, &QAction::triggered, this, &ChatWidgetInput::onPickFile);
    connect(m_emojiMenu, &QMenu::triggered, this, &ChatWidgetInput::onEmojiPicked);
    connect(m_richTextButton, &QToolButton::toggled, this, &ChatWidgetInput::onRichTextToggled);
}

void ChatWidgetInput::onSendClicked()
{
    if (m_isSending) {
        emit stopRequested();
        return;
    }

    const QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty())
        return;

    if (tryApplyCommand(text)) {
        return;
    }

    emit messageSent(text);
    m_inputEdit->clear();
    setSending(true);
}

void ChatWidgetInput::onInputTextChanged(const QString& text)
{
    updateCommandMenu(text);
    emit draftChanged(text);
}

void ChatWidgetInput::onCommandClicked(QListWidgetItem* item)
{
    if (!item)
        return;
    const QString cmd = item->data(Qt::UserRole).toString();
    if (!cmd.isEmpty()) {
        tryApplyCommand(cmd);
    }
}

void ChatWidgetInput::updateCommandMenu(const QString& text)
{
    if (!text.startsWith("/")) {
        m_commandMenu->hide();
        return;
    }

    const QString keyword = text.mid(1).trimmed();
    struct CommandItem {
        const char* cmd;
        const char* label;
    };

    const CommandItem commands[] = {
        { "/trans", "/trans  翻译模式" },
        { "/normal", "/normal 普通模式" }
    };

    m_commandMenu->clear();
    for (const auto& c : commands) {
        const QString cmd = QString::fromUtf8(c.cmd);
        if (keyword.isEmpty() || cmd.mid(1).startsWith(keyword, Qt::CaseInsensitive)) {
            QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(c.label));
            item->setData(Qt::UserRole, cmd);
            m_commandMenu->addItem(item);
        }
    }

    if (m_commandMenu->count() == 0) {
        m_commandMenu->hide();
        return;
    }

    m_commandMenu->setCurrentRow(0);
    positionCommandMenu();
    m_commandMenu->show();
}

bool ChatWidgetInput::tryApplyCommand(const QString& text)
{
    const QString trimmed = text.trimmed();
    const QString command = trimmed.split(' ', Qt::SkipEmptyParts).value(0);

    if (command.compare("/trans", Qt::CaseInsensitive) == 0) {
        applyMode(TranslateMode);
        return true;
    }
    if (command.compare("/normal", Qt::CaseInsensitive) == 0) {
        applyMode(NormalMode);
        return true;
    }
    return false;
}

void ChatWidgetInput::applyMode(InputMode mode)
{
    m_inputMode = mode;
    if (m_inputMode == TranslateMode) {
        m_inputEdit->setPlaceholderText("翻译模式：输入要翻译的内容...");
    } else {
        m_inputEdit->setPlaceholderText("输入消息...");
    }
    m_inputEdit->clear();
    m_inputEdit->setFocus();
    m_commandMenu->hide();
}

void ChatWidgetInput::positionCommandMenu()
{
    const int rowHeight = m_commandMenu->sizeHintForRow(0);
    const int rows = m_commandMenu->count();
    const int maxVisibleRows = qMin(rows, 6);
    const int height = (rowHeight > 0 ? rowHeight : 24) * maxVisibleRows + 2;
    m_commandMenu->setMinimumWidth(m_inputBar->width());
    m_commandMenu->setMaximumWidth(m_inputBar->width());
    m_commandMenu->setMinimumHeight(height);
    m_commandMenu->setMaximumHeight(height);

    const QPoint globalPos = m_inputBar->mapToGlobal(QPoint(0, 0));
    const int y = globalPos.y() - height - 6;
    m_commandMenu->move(globalPos.x(), y);
}

void ChatWidgetInput::setSending(bool sending)
{
    m_isSending = sending;
    m_sendButton->setText(m_isSending ? "停止" : "发送");
}

void ChatWidgetInput::setSendingState(bool sending)
{
    setSending(sending);
}

void ChatWidgetInput::setDraftText(const QString& text)
{
    m_inputEdit->setText(text);
}

QString ChatWidgetInput::draftText() const
{
    return m_inputEdit->text();
}

void ChatWidgetInput::onVoiceClicked()
{
    m_isRecording = !m_isRecording;
    if (m_isRecording) {
        m_voiceButton->setText("停止");
        m_inputEdit->setPlaceholderText("录音中...（占位）");
        m_inputEdit->clear();
        m_commandMenu->hide();
        emit voiceStartRequested();
    } else {
        m_voiceButton->setText("语音");
        m_inputEdit->setPlaceholderText(m_inputMode == TranslateMode ? "翻译模式：输入要翻译的内容..." : "输入消息...");
        emit voiceStopRequested();
    }
}

void ChatWidgetInput::onPickImage()
{
    const QString filter = "Images (*.png *.jpg *.jpeg *.bmp *.gif)";
    const QString path = QFileDialog::getOpenFileName(this, "选择图片", QString(), filter);
    if (path.isEmpty()) {
        return;
    }

    emit messageSent("【图片】" + path);
}

void ChatWidgetInput::onPickFile()
{
    const QString filter = "All Files (*.*)";
    const QString path = QFileDialog::getOpenFileName(this, "选择文件", QString(), filter);
    if (path.isEmpty()) {
        return;
    }

    emit messageSent("【文件】" + path);
}

void ChatWidgetInput::onEmojiPicked(QAction* action)
{
    if (!action) {
        return;
    }
    const QString emoji = action->data().toString();
    if (emoji.isEmpty()) {
        return;
    }
    m_inputEdit->insert(emoji);
    emit emojiSelected(emoji);
}

void ChatWidgetInput::onRichTextToggled(bool checked)
{
    m_richTextEnabled = checked;
    emit richTextToggled(m_richTextEnabled);
}
