#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatwidget/q_chat_widget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMessageSent(const QString &content);


private:
    QChatWidget *m_chatWidget;
    
    // Demo variables
    QString m_lastUserMessage;
    QString m_responseContent;
    int m_streamIndex;
    
private slots:
    void onStartSimulatedReply();
};

#endif // MAINWINDOW_H
