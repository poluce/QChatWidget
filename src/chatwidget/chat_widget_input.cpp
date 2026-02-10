#include "chat_widget_input.h"
#include <QFileDialog>
#include <QEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>
#include <QListWidget>
#include <QMenu>
#include <QResizeEvent>
#include <QSize>
#include <QStyle>
#include <QTextDocument>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtMath>

namespace {
QIcon iconOrFallback(QWidget* widget, const QString& themeName, QStyle::StandardPixmap fallback)
{
    QIcon icon = QIcon::fromTheme(themeName);
    if (icon.isNull() && widget && widget->style()) {
        icon = widget->style()->standardIcon(fallback);
    }
    return icon;
}

void refreshWidgetStyle(QWidget* widget)
{
    if (!widget || !widget->style()) {
        return;
    }
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}
} // namespace

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
    updateInputEditHeight();
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
    m_plusButton->setProperty("role", "icon");
    m_plusButton->setPopupMode(QToolButton::InstantPopup);
    m_plusButton->setAutoRaise(true);
    m_plusButton->setToolTip("更多");
    m_plusButton->setIcon(iconOrFallback(this, QStringLiteral("list-add"), QStyle::SP_FileDialogNewFolder));
    m_plusButton->setText(m_plusButton->icon().isNull() ? "+" : QString());

    m_emojiButton = new QToolButton(m_inputBar);
    m_emojiButton->setObjectName("chatWidgetInputEmojiButton");
    m_emojiButton->setProperty("role", "icon");
    m_emojiButton->setPopupMode(QToolButton::InstantPopup);
    m_emojiButton->setAutoRaise(true);
    m_emojiButton->setToolTip("表情");
    m_emojiButton->setIcon(QIcon::fromTheme(QStringLiteral("face-smile")));
    m_emojiButton->setText(m_emojiButton->icon().isNull() ? "☺" : QString());

    m_voiceButton = new QToolButton(m_inputBar);
    m_voiceButton->setObjectName("chatWidgetInputVoiceButton");
    m_voiceButton->setProperty("role", "icon");
    m_voiceButton->setAutoRaise(true);
    m_voiceButton->setIcon(iconOrFallback(this, QStringLiteral("audio-input-microphone"), QStyle::SP_MediaVolume));
    m_voiceButton->setToolTip("语音输入");
    m_voiceButton->setText(m_voiceButton->icon().isNull() ? "◎" : QString());
    m_inputEdit = new QTextEdit(m_inputBar);
    m_inputEdit->setObjectName("chatWidgetInputEdit");
    m_inputEdit->setPlaceholderText("输入消息...");
    m_inputEdit->setAcceptRichText(false);
    m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_inputEdit->setTabChangesFocus(true);
    m_inputEdit->installEventFilter(this);
    m_richTextButton = new QToolButton(m_inputBar);
    m_richTextButton->setObjectName("chatWidgetInputRichButton");
    m_richTextButton->setCheckable(true);
    m_richTextButton->setProperty("role", "icon");
    m_richTextButton->setAutoRaise(true);
    m_richTextButton->setIcon(QIcon::fromTheme(QStringLiteral("format-text-richtext")));
    m_richTextButton->setText(m_richTextButton->icon().isNull() ? "A" : QString());
    m_richTextButton->setToolTip("富文本");

    m_sendButton = new QToolButton(m_inputBar);
    m_sendButton->setObjectName("chatWidgetInputSendButton");
    m_sendButton->setProperty("role", "icon");
    m_sendButton->setAutoRaise(true);
    m_sendButton->setIcon(iconOrFallback(this, QStringLiteral("mail-send"), QStyle::SP_ArrowForward));
    m_sendButton->setText(m_sendButton->icon().isNull() ? "➤" : QString());
    m_sendButton->setToolTip("发送");

    const QSize toolIconSize(20, 20);
    m_plusButton->setIconSize(toolIconSize);
    m_emojiButton->setIconSize(toolIconSize);
    m_richTextButton->setIconSize(toolIconSize);
    m_voiceButton->setIconSize(toolIconSize);
    m_sendButton->setIconSize(toolIconSize);

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

    QVBoxLayout* inputLayout = new QVBoxLayout(m_inputBar);
    inputLayout->setContentsMargins(10, 6, 10, 6);
    inputLayout->setSpacing(6);
    inputLayout->addWidget(m_inputEdit);

    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setSpacing(8);
    actionLayout->addWidget(m_plusButton);
    actionLayout->addWidget(m_emojiButton);
    actionLayout->addStretch();
    actionLayout->addWidget(m_richTextButton);
    actionLayout->addWidget(m_voiceButton);
    actionLayout->addWidget(m_sendButton);
    inputLayout->addLayout(actionLayout);

    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(m_inputBar);

    connect(m_sendButton, &QToolButton::clicked, this, &ChatWidgetInput::onSendClicked);
    connect(m_inputEdit, &QTextEdit::textChanged, this, [this]() {
        const QString text = m_inputEdit->toPlainText();
        onInputTextChanged(text);
        updateInputEditHeight();
    });
    connect(m_commandMenu, &QListWidget::itemClicked, this, &ChatWidgetInput::onCommandClicked);
    connect(m_voiceButton, &QToolButton::clicked, this, &ChatWidgetInput::onVoiceClicked);
    connect(m_pickImageAction, &QAction::triggered, this, &ChatWidgetInput::onPickImage);
    connect(m_pickFileAction, &QAction::triggered, this, &ChatWidgetInput::onPickFile);
    connect(m_emojiMenu, &QMenu::triggered, this, &ChatWidgetInput::onEmojiPicked);
    connect(m_richTextButton, &QToolButton::toggled, this, &ChatWidgetInput::onRichTextToggled);

    updateInputEditHeight();
    updateVoiceButtonState();
    setSending(false);
}

bool ChatWidgetInput::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_inputEdit && event && event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        const bool isEnter = (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter);
        if (isEnter) {
            const Qt::KeyboardModifiers mods = keyEvent->modifiers();
            if (mods.testFlag(Qt::ControlModifier)) {
                m_inputEdit->insertPlainText("\n");
                return true;
            }
            if (mods == Qt::NoModifier) {
                onSendClicked();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ChatWidgetInput::onSendClicked()
{
    if (m_isSending) {
        emit stopRequested();
        return;
    }

    const QString text = m_inputEdit->toPlainText().trimmed();
    if (text.isEmpty())
        return;

    if (tryApplyCommand(text)) {
        return;
    }

    emit messageSent(text);
    m_inputEdit->clear();
    // 发送态由上层（ChatService/UI）统一驱动，避免本地乐观自锁导致短暂无法连续发送。
    // 这里不再立即切换为 sending=true。
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
    m_sendButton->setProperty("sending", m_isSending);
    m_sendButton->setToolTip(m_isSending ? "停止生成" : "发送");
    m_sendButton->setIcon(m_isSending
                              ? iconOrFallback(this, QStringLiteral("process-stop"), QStyle::SP_MediaStop)
                              : iconOrFallback(this, QStringLiteral("mail-send"), QStyle::SP_ArrowForward));
    m_sendButton->setText(m_sendButton->icon().isNull() ? (m_isSending ? "■" : "➤") : QString());
    refreshWidgetStyle(m_sendButton);
}

void ChatWidgetInput::setSendingState(bool sending)
{
    setSending(sending);
}

void ChatWidgetInput::setDraftText(const QString& text)
{
    m_inputEdit->setPlainText(text);
}

QString ChatWidgetInput::draftText() const
{
    return m_inputEdit->toPlainText();
}

void ChatWidgetInput::onVoiceClicked()
{
    m_isRecording = !m_isRecording;
    if (m_isRecording) {
        m_inputEdit->setPlaceholderText("录音中...（占位）");
        m_inputEdit->clear();
        m_commandMenu->hide();
        emit voiceStartRequested();
    } else {
        m_inputEdit->setPlaceholderText(m_inputMode == TranslateMode ? "翻译模式：输入要翻译的内容..." : "输入消息...");
        emit voiceStopRequested();
    }
    updateVoiceButtonState();
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
    m_inputEdit->insertPlainText(emoji);
    emit emojiSelected(emoji);
}

void ChatWidgetInput::onRichTextToggled(bool checked)
{
    m_richTextEnabled = checked;
    emit richTextToggled(m_richTextEnabled);
}

void ChatWidgetInput::updateInputEditHeight()
{
    if (!m_inputEdit) {
        return;
    }

    constexpr int kMinLines = 1;
    constexpr int kMaxLines = 6;

    const QFontMetrics fm(m_inputEdit->font());
    const int lineHeight = fm.lineSpacing();
    const int contentMargin = static_cast<int>(qRound(m_inputEdit->document()->documentMargin())) * 2;
    const int frame = m_inputEdit->frameWidth() * 2;
    const int minHeight = lineHeight * kMinLines + contentMargin + frame;
    const int maxHeight = lineHeight * kMaxLines + contentMargin + frame;

    const int docHeight = qCeil(m_inputEdit->document()->size().height()) + contentMargin + frame;
    const int targetHeight = qBound(minHeight, docHeight, maxHeight);
    m_inputEdit->setFixedHeight(targetHeight);
    m_inputEdit->setVerticalScrollBarPolicy(docHeight > maxHeight ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);

    // 输入区高度始终由内容驱动，避免在父布局空态时被拉伸占满。
    if (m_inputBar) {
        const int barHeight = m_inputBar->sizeHint().height();
        if (barHeight > 0) {
            setMinimumHeight(barHeight);
            setMaximumHeight(barHeight);
            updateGeometry();
        }
    }
}

void ChatWidgetInput::updateVoiceButtonState()
{
    m_voiceButton->setProperty("recording", m_isRecording);
    m_voiceButton->setToolTip(m_isRecording ? "停止录音" : "语音输入");
    m_voiceButton->setIcon(m_isRecording
                               ? iconOrFallback(this, QStringLiteral("media-playback-stop"), QStyle::SP_MediaStop)
                               : iconOrFallback(this, QStringLiteral("audio-input-microphone"), QStyle::SP_MediaVolume));
    m_voiceButton->setText(m_voiceButton->icon().isNull() ? (m_isRecording ? "■" : "◎") : QString());
    refreshWidgetStyle(m_voiceButton);
}
