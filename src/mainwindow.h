#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QLineEdit>
#include <QPushButton>
#include "chatmodel.h"
#include "chatdelegate.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void onSimulateReply();

private:
    QListView *m_chatView;
    ChatModel *m_model;
    ChatDelegate *m_delegate;
    
    QLineEdit *m_inputEdit;
    QPushButton *m_sendButton;
};

#endif // MAINWINDOW_H