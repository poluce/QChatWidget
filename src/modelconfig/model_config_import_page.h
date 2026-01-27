#ifndef MODEL_CONFIG_IMPORT_PAGE_H
#define MODEL_CONFIG_IMPORT_PAGE_H

#include <QWidget>
#include <QVariantMap>

class QListWidget;
class QStackedWidget;
class QLineEdit;
class QPushButton;
class QFormLayout;

/**
 * @brief 厂商字段定义
 */
struct ModelConfigField {
    QString key;                // 数据键名 (如 "apiKey")
    QString label;              // 界面显示的标签 (如 "API Key")
    QString placeholder;        // 占位符
    QString defaultValue;       // 默认值
    bool isPassword;            // 是否为密码输入
    bool isRequired;            // 是否必填

    ModelConfigField(const QString &k, const QString &l, const QString &p = "", 
                     const QString &d = "", bool pass = false, bool req = true)
        : key(k), label(l), placeholder(p), defaultValue(d), 
          isPassword(pass), isRequired(req) {}
};

/**
 * @brief 厂商完整配置
 */
struct ModelConfigProvider {
    QString id;                 // 唯一标识 (如 "openai")
    QString name;               // 显示名称 (如 "OpenAI")
    QString description;        // 描述
    QList<ModelConfigField> fields;   // 包含的字段列表

    ModelConfigProvider() = default;
    ModelConfigProvider(const QString &i, const QString &n, const QString &d = "")
        : id(i), name(n), description(d) {}
};

class ModelConfigImportPage : public QWidget
{
    Q_OBJECT
public:
    explicit ModelConfigImportPage(QWidget *parent = nullptr);

    /**
     * @brief 动态添加一个模型厂商
     */
    void addProvider(const ModelConfigProvider &provider);

    /**
     * @brief 批量设置厂商列表（会清空现有列表）
     */
    void setProviders(const QList<ModelConfigProvider> &providers);

    /**
     * @brief 设置保存的配置数据（用于回显）
     */
    void setConfigData(const QVariantMap &data);

    /**
     * @brief 获取当前表单的配置
     */
    QVariantMap configData() const;

    /**
     * @brief 应用自定义样式（如果不传则加载内置默认样式）
     */
    void applyStyleSheet(const QString &styleSheet = QString());

signals:
    void importRequested(const QVariantMap &config);
    void cancelled();

private slots:
    void onImportClicked();
    void onTestConnectionClicked();

private:
    void setupUi();
    QWidget* createFormWidget(const ModelConfigProvider &provider);
    QVariantMap collectCurrentConfig() const;
    void updateListWidgetWidth(); // 动态计算并更新列表宽度

    QListWidget *m_providerList = nullptr;
    QStackedWidget *m_detailStack = nullptr;
    
    // 存储各个厂商对应的输入框容器，Key 为 StackedWidget 的索引
    struct FieldWidgets {
        QString providerId;
        QHash<QString, QLineEdit*> inputs; // Key 是字段的 key
    };
    QHash<int, FieldWidgets> m_fieldWidgetsMap;
    QList<ModelConfigProvider> m_providers;

    QPushButton *m_importBtn = nullptr;
    QPushButton *m_testBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
};

#endif // MODEL_CONFIG_IMPORT_PAGE_H
