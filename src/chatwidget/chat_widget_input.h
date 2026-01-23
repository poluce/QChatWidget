#ifndef CHAT_WIDGET_INPUT_H
#define CHAT_WIDGET_INPUT_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QPushButton;
class QToolButton;
class QListWidget;
class QListWidgetItem;
class QResizeEvent;
class QFrame;
class QMenu;
class QAction;

class ChatWidgetInputBase : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidgetInputBase(QWidget *parent = nullptr) : QWidget(parent) {}

signals:
    void messageSent(const QString &content);
};

class ChatWidgetInput : public ChatWidgetInputBase
{
    Q_OBJECT

public:
    explicit ChatWidgetInput(QWidget *parent = nullptr);
    ~ChatWidgetInput();

signals:
    void voiceStartRequested();
    void voiceStopRequested();
    void stopRequested();

public slots:
    void setSendingState(bool sending);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onSendClicked();
    void onInputTextChanged(const QString &text);
    void onCommandClicked(QListWidgetItem *item);
    void onVoiceClicked();
    void onPickImage();
    void onPickFile();

private:
    enum InputMode {
        NormalMode,
        TranslateMode
    };

    void setupUi();
    void updateCommandMenu(const QString &text);
    bool tryApplyCommand(const QString &text);
    void applyMode(InputMode mode);
    void positionCommandMenu();
    void setSending(bool sending);
    QFrame *m_inputBar;
    QLineEdit *m_inputEdit;
    QPushButton *m_sendButton;
    QPushButton *m_voiceButton;
    QToolButton *m_plusButton;
    QMenu *m_plusMenu;
    QAction *m_pickImageAction;
    QAction *m_pickFileAction;
    QListWidget *m_commandMenu;
    InputMode m_inputMode = NormalMode;
    bool m_isRecording = false;
    bool m_isSending = false;
};

#endif // CHAT_WIDGET_INPUT_H
