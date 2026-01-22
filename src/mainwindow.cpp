#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt High Performance Chat");
    resize(400, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    m_model = new ChatModel(this);
    m_delegate = new ChatDelegate(this);
    
    m_chatView = new QListView(this);
    m_chatView->setModel(m_model);
    m_chatView->setItemDelegate(m_delegate);
    
    m_chatView->setSelectionMode(QAbstractItemView::NoSelection);
    m_chatView->setFocusPolicy(Qt::NoFocus);
    m_chatView->setStyleSheet("QListView { background-color: #f5f5f5; border: none; }");
    m_chatView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_chatView->setResizeMode(QListView::Adjust);

    m_inputEdit = new QLineEdit(this);
    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setFixedWidth(60);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(m_chatView);
    
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_inputEdit);
    inputLayout->addWidget(m_sendButton);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    
    mainLayout->addLayout(inputLayout);

    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendClicked);
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendClicked);

    ChatMessage msg1{"User", "你好！这是一个高性能的聊天界面示例。", "", QDateTime::currentDateTime(), false};
    m_model->addMessage(msg1);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onSendClicked()
{
    QString text = m_inputEdit->text().trimmed();
    if (text.isEmpty()) return;

    ChatMessage msg;
    msg.content = text;
    msg.isMine = true;
    msg.timestamp = QDateTime::currentDateTime();
    m_model->addMessage(msg);

    m_inputEdit->clear();
    m_chatView->scrollToBottom();

    QTimer::singleShot(1000, this, &MainWindow::onSimulateReply);
}

void MainWindow::onSimulateReply()
{
    ChatMessage msg;
    msg.content = "我收到了你的消息：" + m_model->data(m_model->index(m_model->rowCount()-1, 0), ChatModel::ContentRole).toString();
    msg.isMine = false;
    msg.timestamp = QDateTime::currentDateTime();
    m_model->addMessage(msg);
    m_chatView->scrollToBottom();
}