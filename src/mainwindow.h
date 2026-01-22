#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qchatwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMessageSent(const QString &content);
    void onSimulateReply();

private:
    QChatWidget *m_chatWidget;
};

#endif // MAINWINDOW_H