#include "chat_widget_input.h"
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>

ChatWidgetInput::ChatWidgetInput(QWidget *parent)
    : ChatWidgetInputBase(parent)
{
    setupUi();
}

ChatWidgetInput::~ChatWidgetInput()
{
}

void ChatWidgetInput::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_commandMenu->isVisible()) {
        positionCommandMenu();
    }
}

void ChatWidgetInput::setupUi()
{
    setObjectName("chatInputRoot");

    m_inputBar = new QFrame(this);
    m_inputBar->setObjectName("inputBar");

    m_plusButton = new QToolButton(m_inputBar);
    m_plusButton->setText("+");
    m_plusButton->setProperty("role", "icon");
    m_plusButton->setFixedWidth(36);
    m_plusButton->setPopupMode(QToolButton::InstantPopup);

    m_voiceButton = new QPushButton("语音", m_inputBar);
    m_voiceButton->setProperty("role", "icon");
    m_voiceButton->setFixedWidth(52);
    m_inputEdit = new QLineEdit(m_inputBar);
    m_inputEdit->setPlaceholderText("输入消息...");
    m_sendButton = new QPushButton("发送", m_inputBar);
    m_sendButton->setProperty("role", "primary");
    m_sendButton->setFixedWidth(64);

    m_plusMenu = new QMenu(this);
    m_pickImageAction = m_plusMenu->addAction("图片");
    m_pickFileAction = m_plusMenu->addAction("文件");
    m_plusButton->setMenu(m_plusMenu);

    m_commandMenu = new QListWidget(this);
    m_commandMenu->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    m_commandMenu->setFocusPolicy(Qt::NoFocus);
    m_commandMenu->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_commandMenu->setSelectionMode(QAbstractItemView::SingleSelection);
    m_commandMenu->setStyleSheet(
        "QListWidget { background: white; border: 1px solid #dfe3ea; border-radius: 10px; }"
        "QListWidget::item { padding: 8px 12px; }"
        "QListWidget::item:selected { background: #e9f1ff; }"
    );
    m_commandMenu->setSpacing(2);

    QGraphicsDropShadowEffect *barShadow = new QGraphicsDropShadowEffect(this);
    barShadow->setBlurRadius(18);
    barShadow->setOffset(0, 3);
    barShadow->setColor(QColor(0, 0, 0, 45));
    m_inputBar->setGraphicsEffect(barShadow);

    QGraphicsDropShadowEffect *menuShadow = new QGraphicsDropShadowEffect(this);
    menuShadow->setBlurRadius(18);
    menuShadow->setOffset(0, 4);
    menuShadow->setColor(QColor(0, 0, 0, 45));
    m_commandMenu->setGraphicsEffect(menuShadow);

    QHBoxLayout *inputLayout = new QHBoxLayout(m_inputBar);
    inputLayout->setContentsMargins(10, 6, 10, 6);
    inputLayout->setSpacing(8);
    inputLayout->addWidget(m_plusButton);
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_voiceButton);
    inputLayout->addWidget(m_sendButton);

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 8, 10, 12);
    rootLayout->addWidget(m_inputBar);

    setStyleSheet(
        "#chatInputRoot { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f7f7f8, stop:1 #eceff3); }"
        "#inputBar { background: #ffffff; border: 1px solid #e5e7eb; border-radius: 14px; }"
        "QPushButton[role=\"icon\"] { background: #f4f6f9; border: 1px solid #e1e4ea; border-radius: 10px; padding: 6px 10px; color: #374151; }"
        "QPushButton[role=\"icon\"]:hover { background: #eef2f7; }"
        "QPushButton[role=\"icon\"]:pressed { background: #e2e8f0; }"
        "QToolButton[role=\"icon\"] { background: #f4f6f9; border: 1px solid #e1e4ea; border-radius: 10px; padding: 4px 8px; color: #374151; }"
        "QToolButton[role=\"icon\"]:hover { background: #eef2f7; }"
        "QToolButton[role=\"icon\"]:pressed { background: #e2e8f0; }"
        "QPushButton[role=\"primary\"] { background: #4b7bec; color: white; border: none; border-radius: 12px; padding: 6px 14px; }"
        "QPushButton[role=\"primary\"]:hover { background: #3b6fe0; }"
        "QPushButton[role=\"primary\"]:pressed { background: #2f62d6; }"
        "QLineEdit { background: transparent; border: none; padding: 6px 6px; color: #111827; }"
        "QMenu { background: white; border: 1px solid #dfe3ea; border-radius: 10px; padding: 6px; }"
        "QMenu::item { padding: 6px 16px; border-radius: 6px; }"
        "QMenu::item:selected { background: #e9f1ff; }"
    );

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWidgetInput::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &ChatWidgetInput::onSendClicked);
    connect(m_inputEdit, &QLineEdit::textChanged, this, &ChatWidgetInput::onInputTextChanged);
    connect(m_commandMenu, &QListWidget::itemClicked, this, &ChatWidgetInput::onCommandClicked);
    connect(m_voiceButton, &QPushButton::clicked, this, &ChatWidgetInput::onVoiceClicked);
    connect(m_pickImageAction, &QAction::triggered, this, &ChatWidgetInput::onPickImage);
    connect(m_pickFileAction, &QAction::triggered, this, &ChatWidgetInput::onPickFile);
}

void ChatWidgetInput::onSendClicked()
{
    if (m_isSending) {
        emit stopRequested();
        return;
    }

    const QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    if (tryApplyCommand(text)) {
        return;
    }

    emit messageSent(text);
    m_inputEdit->clear();
    setSending(true);
}

void ChatWidgetInput::onInputTextChanged(const QString &text)
{
    updateCommandMenu(text);
}

void ChatWidgetInput::onCommandClicked(QListWidgetItem *item)
{
    if (!item) return;
    const QString cmd = item->data(Qt::UserRole).toString();
    if (!cmd.isEmpty()) {
        tryApplyCommand(cmd);
    }
}

void ChatWidgetInput::updateCommandMenu(const QString &text)
{
    if (!text.startsWith("/")) {
        m_commandMenu->hide();
        return;
    }

    const QString keyword = text.mid(1).trimmed();
    struct CommandItem {
        const char *cmd;
        const char *label;
    };

    const CommandItem commands[] = {
        { "/trans",  "/trans  翻译模式" },
        { "/normal", "/normal 普通模式" }
    };

    m_commandMenu->clear();
    for (const auto &c : commands) {
        const QString cmd = QString::fromUtf8(c.cmd);
        if (keyword.isEmpty() || cmd.mid(1).startsWith(keyword, Qt::CaseInsensitive)) {
            QListWidgetItem *item = new QListWidgetItem(QString::fromUtf8(c.label));
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

bool ChatWidgetInput::tryApplyCommand(const QString &text)
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
    m_commandMenu->setFixedWidth(m_inputBar->width());
    m_commandMenu->setFixedHeight(height);

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
        m_inputEdit->setPlaceholderText(m_inputMode == TranslateMode
                                            ? "翻译模式：输入要翻译的内容..."
                                            : "输入消息...");
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
