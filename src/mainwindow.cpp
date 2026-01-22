#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt High Performance Chat");
    resize(400, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    m_chatWidget = new QChatWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(m_chatWidget);

    connect(m_chatWidget, &QChatWidget::messageSent, this, &MainWindow::onMessageSent);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onMessageSent(const QString &content)
{
    // Simulate auto-reply logic
    Q_UNUSED(content);
    QTimer::singleShot(1000, this, &MainWindow::onSimulateReply);
}

void MainWindow::onSimulateReply()
{
    QString replyContent = "我收到了你的消息"; // Generic reply for now as model access is encapsulated
    m_chatWidget->addMessage(replyContent, false, "AutoReply");
}