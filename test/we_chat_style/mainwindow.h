#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatwidget/chat_widget.h"

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMessageSent(const QString &content);
    void onStartSimulatedReply();
    void onToggleTheme();
    void onThemeChanged();

private:
    ChatWidget *m_chatWidget;
    QPushButton *m_themeButton;
    
    // Demo variables
    QString m_lastUserMessage;
    QString m_responseContent;
    int m_streamIndex;
};

#endif // MAINWINDOW_H
