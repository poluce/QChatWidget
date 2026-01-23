#include "mainwindow.h"
#include "common/theme_manager.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Qt AI Chat Demo - 主题切换");
  resize(500, 700);

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  // 顶部工具栏
  QHBoxLayout *toolbarLayout = new QHBoxLayout();
  toolbarLayout->setContentsMargins(10, 10, 10, 5);

  m_themeButton = new QPushButton("🌙 深色主题", this);
  m_themeButton->setFixedWidth(120);
  toolbarLayout->addStretch();
  toolbarLayout->addWidget(m_themeButton);

  connect(m_themeButton, &QPushButton::clicked, this,
          &MainWindow::onToggleTheme);

  m_chatWidget = new ChatWidget(this);
  m_chatWidget->applyStyleSheetFile("chat_widget.qss");

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addLayout(toolbarLayout);
  mainLayout->addWidget(m_chatWidget);

  connect(m_chatWidget, &ChatWidget::messageSent, this,
          &MainWindow::onMessageSent);
  connect(m_chatWidget, &ChatWidget::stopRequested, this,
          &MainWindow::onStopRequested);
  connect(ThemeManager::instance(), &ThemeManager::themeChanged, this,
          &MainWindow::onThemeChanged);

  // Welcome message with Markdown
  m_chatWidget->addMessage(
      "**Hello!** I am an AI Assistant.\nI support *Markdown* rendering and "
      "`streaming` output.\n\n点击右上角按钮可切换 **深色/浅色** 主题！",
      false, "AI");
}

MainWindow::~MainWindow() {}

void MainWindow::onMessageSent(const QString &content) {
  m_lastUserMessage = content;
  QTimer::singleShot(600, this, &MainWindow::onStartSimulatedReply);
}

void MainWindow::onStartSimulatedReply() {
  m_chatWidget->addMessage("", false, "AI");

  m_responseContent = m_lastUserMessage;
  m_streamIndex = 0;

  if (m_responseTimer) {
    m_responseTimer->stop();
    m_responseTimer->deleteLater();
  }

  m_responseTimer = new QTimer(this);
  connect(m_responseTimer, &QTimer::timeout, this, [this]() {
    if (m_streamIndex < m_responseContent.length()) {
      int chunk = qMin(3, m_responseContent.length() - m_streamIndex);
      QString textChunk = m_responseContent.mid(m_streamIndex, chunk);
      m_chatWidget->streamOutput(textChunk);
      m_streamIndex += chunk;
    } else {
      onStopRequested();
    }
  });
  m_responseTimer->start(30);
}

void MainWindow::onStopRequested() {
  if (m_responseTimer) {
    m_responseTimer->stop();
    m_responseTimer->deleteLater();
    m_responseTimer = nullptr;
  }
  m_chatWidget->setSendingState(false);
}

void MainWindow::onToggleTheme() {
  ThemeManager *tm = ThemeManager::instance();
  if (tm->currentTheme() == ThemeManager::Light) {
    tm->setTheme(ThemeManager::Dark);
  } else {
    tm->setTheme(ThemeManager::Light);
  }
}

void MainWindow::onThemeChanged() {
  ThemeManager *tm = ThemeManager::instance();
  bool isDark = (tm->currentTheme() == ThemeManager::Dark);

  // 更新按钮文字
  m_themeButton->setText(isDark ? "☀️ 浅色主题" : "🌙 深色主题");

  // 应用 ChatWidget 样式
  ChatWidgetDelegate::Style chatStyle;
  auto themeStyle = tm->chatWidgetStyle();
  chatStyle.avatarSize = themeStyle.avatarSize;
  chatStyle.margin = themeStyle.margin;
  chatStyle.bubblePadding = themeStyle.bubblePadding;
  chatStyle.bubbleRadius = themeStyle.bubbleRadius;
  chatStyle.myBubbleColor = themeStyle.myBubbleColor;
  chatStyle.otherBubbleColor = themeStyle.otherBubbleColor;
  chatStyle.myAvatarColor = themeStyle.myAvatarColor;
  chatStyle.otherAvatarColor = themeStyle.otherAvatarColor;
  chatStyle.myTextColor = themeStyle.myTextColor;
  chatStyle.otherTextColor = themeStyle.otherTextColor;
  chatStyle.messageFont = themeStyle.messageFont;
  chatStyle.avatarFont = themeStyle.avatarFont;

  m_chatWidget->setDelegateStyle(chatStyle);

  // 更新窗口背景
  if (isDark) {
    setStyleSheet("QMainWindow { background-color: #1a1a1a; } "
                  "QPushButton { background-color: #333; color: #eee; border: "
                  "none; padding: 8px; border-radius: 4px; } "
                  "QPushButton:hover { background-color: #444; }");
  } else {
    setStyleSheet("QMainWindow { background-color: #f5f5f5; } "
                  "QPushButton { background-color: #fff; color: #333; border: "
                  "1px solid #ddd; padding: 8px; border-radius: 4px; } "
                  "QPushButton:hover { background-color: #eee; }");
  }
}
