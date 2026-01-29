#include "model_config_import_page.h"
#include "qss_utils.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

ModelConfigImportPage::ModelConfigImportPage(QWidget* parent) : QWidget(parent)
{
    setupUi();
    applyStyleSheet(); // 加载默认样式
}

void ModelConfigImportPage::applyStyleSheet(const QString& styleSheet)
{
    const QString combinedStyle = QssUtils::buildCombinedStyleSheet(
        "model_config_import_page.qss",
        styleSheet);
    this->setStyleSheet(combinedStyle);
}

void ModelConfigImportPage::addProvider(const ModelConfigProvider& provider)
{
    m_providers << provider;
    m_providerList->addItem(provider.name);
    m_detailStack->addWidget(createFormWidget(provider));

    updateListWidgetWidth(); // 动态调整宽度
}

void ModelConfigImportPage::setProviders(const QList<ModelConfigProvider>& providers)
{
    m_providers = providers;
    m_providerList->clear();

    // 清理并重新创建 stack
    while (m_detailStack->count() > 0) {
        QWidget* w = m_detailStack->widget(0);
        m_detailStack->removeWidget(w);
        w->deleteLater();
    }
    m_fieldWidgetsMap.clear();

    for (const auto& p : providers) {
        m_providerList->addItem(p.name);
        m_detailStack->addWidget(createFormWidget(p));
    }

    if (m_providerList->count() > 0) {
        m_providerList->setCurrentRow(0);
    }

    updateListWidgetWidth(); // 批量添加后统一调整
    setDirty(false);
    clearFieldErrors();
    setTestStatus(TestStatus::Idle);
}

void ModelConfigImportPage::setConfigData(const QVariantMap& data)
{
    QString providerId = data.value("providerId").toString();
    if (providerId.isEmpty())
        return;

    for (int i = 0; i < m_providers.size(); ++i) {
        if (m_providers[i].id == providerId) {
            m_providerList->setCurrentRow(i);
            const auto& widgets = m_fieldWidgetsMap[i].inputs;
            for (auto it = widgets.begin(); it != widgets.end(); ++it) {
                if (data.contains(it.key())) {
                    it.value()->setText(data[it.key()].toString());
                }
            }
            break;
        }
    }
    setDirty(false);
    clearFieldErrors();
    setTestStatus(TestStatus::Idle);
}

QVariantMap ModelConfigImportPage::configData() const
{
    return collectCurrentConfig();
}

void ModelConfigImportPage::setupUi()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 外层不留边距，由内部容器控制
    mainLayout->setSpacing(0);

    // 1. 左侧容器：解决 QListWidget 在 QSS 中设置 padding 导致文字裁剪的问题
    QWidget* leftContainer = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(10, 10, 0, 10); // 左、上、下边缘留白

    m_providerList = new QListWidget(leftContainer);
    m_providerList->setObjectName("providerList");

    // 按照最佳实践配置：禁用换行，按需显示滚动条，内容自动调整策略
    m_providerList->setWordWrap(false);
    m_providerList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_providerList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_providerList->setTextElideMode(Qt::ElideNone); // 配合禁止裁剪

    leftLayout->addWidget(m_providerList);

    // 2. 右侧容器
    QWidget* rightContainer = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 10, 10, 10); // 上、右、下边缘留白

    m_detailStack = new QStackedWidget(rightContainer);
    m_detailStack->setObjectName("detailStack");

    m_importBtn = new QPushButton(tr("导入并保存"), rightContainer);
    m_importBtn->setObjectName("importBtn");
    m_testBtn = new QPushButton(tr("验证连接"), rightContainer);
    m_testBtn->setObjectName("testBtn");
    m_cancelBtn = new QPushButton(tr("取消"), rightContainer);
    m_cancelBtn->setObjectName("cancelBtn");
    m_importFileBtn = new QPushButton(tr("从文件导入"), rightContainer);
    m_importFileBtn->setObjectName("importFileBtn");
    m_exportBtn = new QPushButton(tr("导出配置"), rightContainer);
    m_exportBtn->setObjectName("exportBtn");
    m_resetBtn = new QPushButton(tr("重置为默认"), rightContainer);
    m_resetBtn->setObjectName("resetBtn");
    m_testStatusLabel = new QLabel(tr("未验证"), rightContainer);
    m_testStatusLabel->setObjectName("testStatusLabel");
    setTestStatus(TestStatus::Idle);

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addWidget(m_testStatusLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_importFileBtn);
    bottomLayout->addWidget(m_exportBtn);
    bottomLayout->addWidget(m_resetBtn);
    bottomLayout->addWidget(m_testBtn);
    bottomLayout->addWidget(m_cancelBtn);
    bottomLayout->addWidget(m_importBtn);

    rightLayout->addWidget(m_detailStack);
    rightLayout->addLayout(bottomLayout);

    // 3. 使用 QSplitter 连接两侧，实现用户可调、响应式缩放
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftContainer);
    splitter->addWidget(rightContainer);
    splitter->setStretchFactor(0, 1); // 左侧比例
    splitter->setStretchFactor(1, 3); // 右侧比例
    splitter->setHandleWidth(1);      // 细分割线

    mainLayout->addWidget(splitter);

    connect(m_providerList, &QListWidget::currentRowChanged, m_detailStack, &QStackedWidget::setCurrentIndex);
    connect(m_importBtn, &QPushButton::clicked, this, &ModelConfigImportPage::onImportClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ModelConfigImportPage::cancelled);
    connect(m_testBtn, &QPushButton::clicked, this, &ModelConfigImportPage::onTestConnectionClicked);
    connect(m_importFileBtn, &QPushButton::clicked, this, &ModelConfigImportPage::onImportFromFileClicked);
    connect(m_exportBtn, &QPushButton::clicked, this, &ModelConfigImportPage::onExportClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &ModelConfigImportPage::onResetClicked);
}

QWidget* ModelConfigImportPage::createFormWidget(const ModelConfigProvider& provider)
{
    QWidget* container = new QWidget();
    QFormLayout* layout = new QFormLayout(container);
    layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    layout->setLabelAlignment(Qt::AlignRight);

    QLabel* header = new QLabel(QString("<h2>%1 %2</h2>").arg(provider.name).arg(tr("配置")));
    layout->addRow(header);

    FieldWidgets fieldWidgets;
    fieldWidgets.providerId = provider.id;

    for (const auto& f : provider.fields) {
        QLineEdit* edit = new QLineEdit();
        if (f.isPassword)
            edit->setEchoMode(QLineEdit::Password);
        edit->setPlaceholderText(f.placeholder);
        edit->setText(f.defaultValue);

        layout->addRow(f.label + ":", edit);
        fieldWidgets.inputs.insert(f.key, edit);

        QLabel* errorLabel = new QLabel();
        errorLabel->setObjectName("fieldError");
        errorLabel->setVisible(false);
        errorLabel->setWordWrap(true);
        layout->addRow(QString(), errorLabel);
        fieldWidgets.errors.insert(f.key, errorLabel);

        const QString fieldKey = f.key;
        connect(edit, &QLineEdit::textChanged, this, [this, fieldKey]() {
            setDirty(true);
            setFieldError(fieldKey, QString());
        });
    }

    int index = m_detailStack->count();
    m_fieldWidgetsMap.insert(index, fieldWidgets);

    return container;
}

void ModelConfigImportPage::onImportClicked()
{
    QVariantMap config = collectCurrentConfig();
    if (config.isEmpty())
        return;

    // 基础必填验证逻辑
    int index = m_providerList->currentRow();
    const auto& provider = m_providers[index];
    for (const auto& f : provider.fields) {
        if (f.isRequired && config[f.key].toString().isEmpty()) {
            QMessageBox::warning(this, tr("验证失败"), QString(tr("%1 不能为空")).arg(f.label));
            return;
        }
    }

    emit importRequested(config);
}

void ModelConfigImportPage::onTestConnectionClicked()
{
    QVariantMap config = collectCurrentConfig();
    if (config.isEmpty())
        return;
    setTestStatus(TestStatus::Testing);
    emit testConnectionRequested(config);
}

void ModelConfigImportPage::onImportFromFileClicked()
{
    emit importFromFileRequested();
}

void ModelConfigImportPage::onExportClicked()
{
    QVariantMap config = collectCurrentConfig();
    if (config.isEmpty())
        return;
    emit exportRequested(config);
}

void ModelConfigImportPage::onResetClicked()
{
    int index = m_providerList->currentRow();
    if (index < 0 || index >= m_providers.size())
        return;
    const auto& provider = m_providers[index];
    const auto& widgets = m_fieldWidgetsMap[index];
    for (const auto& f : provider.fields) {
        if (widgets.inputs.contains(f.key)) {
            widgets.inputs[f.key]->setText(f.defaultValue);
        }
    }
    clearFieldErrors();
    setDirty(true);
    emit resetRequested(provider.id);
}

QVariantMap ModelConfigImportPage::collectCurrentConfig() const
{
    int index = m_providerList->currentRow();
    if (index < 0 || index >= m_providers.size())
        return QVariantMap();

    const auto& provider = m_providers[index];
    const auto& widgets = m_fieldWidgetsMap[index];

    QVariantMap config;
    config["providerId"] = provider.id;
    config["providerName"] = provider.name; // 额外补充名称，方便显示

    // 遍历当前厂商定义的所有字段并从 UI 提取值
    for (auto it = widgets.inputs.begin(); it != widgets.inputs.end(); ++it) {
        config[it.key()] = it.value()->text();
    }

    return config;
}

void ModelConfigImportPage::updateListWidgetWidth()
{
    if (!m_providerList)
        return;

    int maxWidth = 0;
    QFontMetrics fm(m_providerList->font());

    for (int i = 0; i < m_providerList->count(); ++i) {
        QString text = m_providerList->item(i)->text();
        // Qt 5.11+ 使用 horizontalAdvance，它能更准确地测量文本宽度
        int width = fm.horizontalAdvance(text);
        maxWidth = qMax(maxWidth, width);
    }

    // 预留空间：考虑图标、Padding (15px * 2)、外边距 (8px * 2) 以及滚动条预留
    // 按照用户建议预留 60px 是稳健的方案
    m_providerList->setMinimumWidth(maxWidth + 60);
}

void ModelConfigImportPage::setTestStatus(TestStatus status, const QString& message)
{
    m_testStatus = status;
    if (!m_testStatusLabel)
        return;
    QString text;
    QString statusKey;
    switch (status) {
    case TestStatus::Idle:
        text = tr("未验证");
        statusKey = "idle";
        break;
    case TestStatus::Testing:
        text = tr("验证中...");
        statusKey = "testing";
        break;
    case TestStatus::Success:
        text = tr("验证成功");
        statusKey = "success";
        break;
    case TestStatus::Failed:
        text = tr("验证失败");
        statusKey = "failed";
        break;
    }
    if (!message.trimmed().isEmpty()) {
        text += tr("：") + message;
    }
    m_testStatusLabel->setText(text);
    m_testStatusLabel->setProperty("status", statusKey);
    m_testStatusLabel->style()->unpolish(m_testStatusLabel);
    m_testStatusLabel->style()->polish(m_testStatusLabel);
}

void ModelConfigImportPage::clearFieldErrors()
{
    int index = m_providerList->currentRow();
    if (index < 0 || index >= m_providers.size())
        return;
    auto& widgets = m_fieldWidgetsMap[index];
    for (auto it = widgets.errors.begin(); it != widgets.errors.end(); ++it) {
        it.value()->clear();
        it.value()->setVisible(false);
    }
}

void ModelConfigImportPage::setFieldError(const QString& fieldKey, const QString& message)
{
    setFieldError(QString(), fieldKey, message);
}

void ModelConfigImportPage::setFieldError(const QString& providerId, const QString& fieldKey, const QString& message)
{
    int index = providerId.isEmpty() ? m_providerList->currentRow() : providerIndexForId(providerId);
    if (index < 0 || index >= m_providers.size())
        return;
    auto& widgets = m_fieldWidgetsMap[index];
    QLabel* label = widgets.errors.value(fieldKey, nullptr);
    if (!label)
        return;
    label->setText(message);
    label->setVisible(!message.trimmed().isEmpty());
}

void ModelConfigImportPage::setDirty(bool dirty)
{
    if (m_dirty == dirty)
        return;
    m_dirty = dirty;
    emit dirtyChanged(m_dirty);
}

int ModelConfigImportPage::providerIndexForId(const QString& providerId) const
{
    if (providerId.isEmpty())
        return -1;
    for (int i = 0; i < m_providers.size(); ++i) {
        if (m_providers[i].id == providerId) {
            return i;
        }
    }
    return -1;
}
