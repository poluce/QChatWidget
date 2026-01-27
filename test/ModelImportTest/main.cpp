#include <QApplication>
#include "modelconfig/model_config_import_page.h"
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ModelConfigImportPage w;
    w.setWindowTitle("Model Import Test (Library Mode)");
    w.resize(800, 500);

    // 1. 初始化核心厂商列表
    QList<ModelConfigProvider> defaults;
    
    // DeepSeek
    ModelConfigProvider deepseek{"deepseek", "DeepSeek", "中国高性能 AI 模型"};
    deepseek.fields << ModelConfigField{"apiKey", "API 密钥", "sk-...", "", true, true};
    deepseek.fields << ModelConfigField{"modelId", "模型名称", "deepseek-chat", "deepseek-chat"};
    deepseek.fields << ModelConfigField{"baseUrl", "接口地址", "https://api.deepseek.com", "https://api.deepseek.com"};

    // OpenAI
    ModelConfigProvider openai{"openai", "OpenAI", "全球领先的 AI 语言模型"};
    openai.fields << ModelConfigField{"apiKey", "API 密钥", "sk-...", "", true, true};
    openai.fields << ModelConfigField{"modelId", "模型名称", "gpt-4o", "gpt-4o"};
    openai.fields << ModelConfigField{"baseUrl", "接口地址", "https://api.openai.com/v1", "https://api.openai.com/v1"};

    // Claude
    ModelConfigProvider claude{"claude", "Claude", "Anthropic 强大的 AI 模型"};
    claude.fields << ModelConfigField{"apiKey", "API 密钥", "sk-ant-...", "", true, true};
    claude.fields << ModelConfigField{"modelId", "模型名称", "claude-3-5-sonnet", "claude-3-5-sonnet"};
    claude.fields << ModelConfigField{"baseUrl", "接口地址", "https://api.anthropic.com/v1", "https://api.anthropic.com/v1"};

    // Ollama
    ModelConfigProvider ollama{"ollama", "Ollama", "本地运行的各类型开源模型"};
    ollama.fields << ModelConfigField{"modelId", "模型名称", "llama3", "llama3"};
    ollama.fields << ModelConfigField{"baseUrl", "接口地址", "http://localhost:11434", "http://localhost:11434"};

    defaults << deepseek << openai << claude << ollama;
    w.setProviders(defaults);

    // 2. 动态追加谷歌 Gemini
    ModelConfigProvider gemini{"gemini", "Gemini", "Google 强大的 AI 服务"};
    gemini.fields << ModelConfigField{"apiKey", "API 密钥", "在此输入密钥", "", true, true};
    gemini.fields << ModelConfigField{"modelId", "模型名称", "gemini-1.5-pro", "gemini-1.5-pro"};
    gemini.fields << ModelConfigField{"baseUrl", "接口地址", "https://generativelanguage.googleapis.com", ""};
    w.addProvider(gemini);

    QObject::connect(&w, &ModelConfigImportPage::importRequested, [](const QVariantMap &config) {
        qDebug() << "Import Requested with dynamic config:" << config;
    });

    QObject::connect(&w, &ModelConfigImportPage::cancelled, []() {
        qDebug() << "Import Cancelled";
    });

    QObject::connect(&w, &ModelConfigImportPage::testConnectionRequested,
                     [&w](const QVariantMap &config) {
        qDebug() << "Test Connection Requested:" << config;
        // UI-only demo: 模拟异步验证结果回调
        QTimer::singleShot(800, [&w]() {
            w.setTestStatus(ModelConfigImportPage::TestStatus::Success,
                            QStringLiteral("模拟成功"));
        });
    });

    QObject::connect(&w, &ModelConfigImportPage::importFromFileRequested, []() {
        qDebug() << "Import From File Requested";
    });

    QObject::connect(&w, &ModelConfigImportPage::exportRequested,
                     [](const QVariantMap &config) {
        qDebug() << "Export Requested:" << config;
    });

    QObject::connect(&w, &ModelConfigImportPage::resetRequested,
                     [](const QString &providerId) {
        qDebug() << "Reset Requested for provider:" << providerId;
    });

    QObject::connect(&w, &ModelConfigImportPage::dirtyChanged, [](bool dirty) {
        qDebug() << "Dirty Changed:" << dirty;
    });

    w.show();

    return a.exec();
}
