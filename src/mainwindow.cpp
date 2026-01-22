#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Qt AI Chat Demo");
    resize(500, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    m_chatWidget = new QChatWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(m_chatWidget);

    connect(m_chatWidget, &QChatWidget::messageSent, this, &MainWindow::onMessageSent);
    
    // Welcome message with Markdown
    m_chatWidget->addMessage("**Hello!** I am an AI Assistant.\nI support *Markdown* rendering and `streaming` output.\n\nTry sending a message!", false, "AI");
}

MainWindow::~MainWindow()
{
}

void MainWindow::onMessageSent(const QString &content)
{
    Q_UNUSED(content);
    // Simulate thinking delay
    QTimer::singleShot(600, this, &MainWindow::onStartSimulatedReply);
}

void MainWindow::onStartSimulatedReply()
{
    // 1. Create an empty message first
    m_chatWidget->addMessage("", false, "AI");
    
    // 2. Start streaming content
    m_responseContent = "I received your message. Here is a **Markdown** list:\n"
                        "- Item 1\n- Item 2\n- `Code Block`\n\n"
                        "And here is a table:\n\n"
                        "| Header 1 | Header 2 |\n"
                        "| -------- | -------- |\n"
                        "| Cell 1   | Cell 2   |\n";
    m_streamIndex = 0;
    
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this, timer]() {
        if (m_streamIndex < m_responseContent.length()) {
            // Append 1-3 characters at a time to simulate typing
            int chunk = qMin(3, m_responseContent.length() - m_streamIndex);
            QString textChunk = m_responseContent.mid(m_streamIndex, chunk);
            m_chatWidget->streamOutput(textChunk);
            m_streamIndex += chunk;
        } else {
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start(30); // 30ms delay between chunks
}