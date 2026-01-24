#ifndef CHAT_WIDGET_H
#define CHAT_WIDGET_H

#include "chat_widget_delegate.h"
#include <QString>
#include <QWidget>

class ChatWidgetView;
class ChatWidgetInputBase;
class QTimer;

class ChatWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ChatWidget(QWidget *parent = nullptr);
  ~ChatWidget();

  // API: 添加消息
  void addMessage(const QString &content, bool isMine,
                  const QString &sender = "User");

  // API: 流式输出（追加内容到最后一条消息）
  void streamOutput(const QString &content);

  // API: 移除最后一条消息
  void removeLastMessage();
  bool applyStyleSheetFile(const QString &fileNameOrPath);
  void setDelegateStyle(const ChatWidgetDelegate::Style &style);
  ChatWidgetDelegate::Style delegateStyle() const;
  void setInputWidget(class ChatWidgetInputBase *widget);
  class ChatWidgetInputBase *inputWidget() const;
  void setSendingState(bool sending);

  // API: 模拟 AI 自动流式回复（组件内部管理定时器）
  void startSimulatedStreaming(const QString &content, int interval = 30);

signals:
  // API: 消息发送信号
  void messageSent(const QString &content);
  void stopRequested();

private slots:
  void onInputMessageSent(const QString &content);
  void onStreamingTimeout();

private:
  void setupUi();

  class QVBoxLayout *m_mainLayout;
  class ChatWidgetView *m_viewWidget;
  class ChatWidgetInputBase *m_inputWidget;
  bool m_isSending = false;

  QTimer *m_streamingTimer = nullptr;
  QString m_streamingContent;
  int m_streamingIndex = 0;
};

#endif // CHAT_WIDGET_H
