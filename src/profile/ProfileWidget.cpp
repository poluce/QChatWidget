#include "ProfileWidget.h"

#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

// ==========================================
// DetailItemWidget 实现
// ==========================================
DetailItemWidget::DetailItemWidget(const QString& key, QWidget* parent)
    : QWidget(parent), m_key(key) { }

void DetailItemWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (rect().contains(event->pos())) {
        emit itemClicked(m_key);
    }
    QWidget::mouseReleaseEvent(event);
}

// ==========================================
// ProfileWidget 实现
// ==========================================

namespace {
QPixmap buildRoundedAvatar(const QPixmap& source, int size)
{
    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap rounded(size, size);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaled);
    return rounded;
}
} // namespace

ProfileWidget::ProfileWidget(QWidget* parent) : QWidget(parent)
{
    setupUi();
    setupStyle();

    connect(m_msgButton, &QPushButton::clicked, this, &ProfileWidget::messageClicked);
    connect(m_moreButton, &QToolButton::clicked, this, &ProfileWidget::moreOptionsClicked);

    this->setFixedSize(360, 640);
    this->setWindowTitle("个人信息");
}

void ProfileWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 固定内容区域
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setObjectName("ContentWidget");
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 20);
    contentLayout->setSpacing(0);

    // --- Header ---
    QWidget* headerWidget = new QWidget();
    headerWidget->setObjectName("HeaderWidget");
    headerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 30, 20, 30);
    headerLayout->setSpacing(15);
    headerLayout->setAlignment(Qt::AlignVCenter);

    m_avatarLabel = new QLabel();
    m_avatarLabel->setFixedSize(64, 64);
    m_avatarLabel->setScaledContents(true);
    m_avatarLabel->setObjectName("AvatarLabel");
    QPixmap defaultAvatar(64, 64);
    defaultAvatar.fill(QColor(220, 220, 220));
    m_avatarLabel->setPixmap(buildRoundedAvatar(defaultAvatar, 64));

    QVBoxLayout* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(5);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setAlignment(Qt::AlignVCenter);

    m_nameLabel = new QLabel("agentname");
    m_nameLabel->setObjectName("NameLabel");
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_idLabel = new QLabel("TmId: --");
    m_idLabel->setObjectName("IdLabel");
    m_idLabel->setWordWrap(false);
    m_idLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_idLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_moreButton = new QToolButton();
    m_moreButton->setText("•••");
    m_moreButton->setCursor(Qt::PointingHandCursor);
    m_moreButton->setObjectName("MoreButton");
    m_moreButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    infoLayout->addWidget(m_nameLabel);
    infoLayout->addWidget(m_idLabel);

    headerLayout->addWidget(m_avatarLabel, 0, Qt::AlignVCenter);
    headerLayout->addLayout(infoLayout, 1);
    headerLayout->addWidget(m_moreButton, 0, Qt::AlignTop);

    QFrame* headerGap = new QFrame();
    headerGap->setFixedHeight(10);
    headerGap->setObjectName("GapFrame");

    // --- Detail Group ---
    QLabel* groupTitle = new QLabel(QStringLiteral("详细信息"));
    groupTitle->setObjectName("DetailGroupTitle");
    groupTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QFrame* detailGroup = new QFrame();
    detailGroup->setObjectName("DetailGroup");
    detailGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_detailListLayout = new QVBoxLayout(detailGroup);
    m_detailListLayout->setContentsMargins(16, 8, 16, 8);
    m_detailListLayout->setSpacing(0);

    contentLayout->addWidget(headerWidget);
    contentLayout->addWidget(headerGap);
    contentLayout->addWidget(groupTitle);
    contentLayout->addWidget(detailGroup);
    contentLayout->addStretch();

    mainLayout->addWidget(contentWidget, 1);

    // --- Bottom Bar ---
    QWidget* bottomBar = new QWidget();
    bottomBar->setObjectName("BottomBar");
    QHBoxLayout* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(20, 15, 20, 15);

    m_msgButton = new QPushButton();
    m_msgButton->setText(" 发消息");
    m_msgButton->setIcon(
        this->style()->standardIcon(QStyle::SP_DialogApplyButton));
    m_msgButton->setCursor(Qt::PointingHandCursor);
    m_msgButton->setObjectName("MsgButton");
    m_msgButton->setFixedHeight(45);

    bottomLayout->addWidget(m_msgButton);
    mainLayout->addWidget(bottomBar);
}

void ProfileWidget::addDetailItem(const QString& title, const QString& content, bool isEditable)
{
    if (!m_detailListLayout)
        return;
    QWidget* item = createDetailWidget(title, content, isEditable);
    m_detailListLayout->addWidget(item);
}

void ProfileWidget::addSeparator()
{
    if (!m_detailListLayout)
        return;
    m_detailListLayout->addWidget(createSeparatorFrame());
}

void ProfileWidget::clearDetails()
{
    if (!m_detailListLayout)
        return;

    QLayoutItem* item;
    while ((item = m_detailListLayout->takeAt(0)) != nullptr) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
}

QWidget* ProfileWidget::createDetailWidget(const QString& title, const QString& content, bool isEditable)
{
    QWidget* item;
    if (isEditable) {
        DetailItemWidget* interactiveItem = new DetailItemWidget(title);
        connect(interactiveItem, &DetailItemWidget::itemClicked, this, &ProfileWidget::detailItemClicked);
        item = interactiveItem;
        item->setCursor(Qt::PointingHandCursor);
    } else {
        item = new QWidget();
    }

    QHBoxLayout* layout = new QHBoxLayout(item);
    layout->setContentsMargins(0, 10, 0, 10);
    layout->setSpacing(12);
    layout->setAlignment(Qt::AlignTop);

    QLabel* titleLbl = new QLabel(title);
    titleLbl->setFixedWidth(70);
    titleLbl->setObjectName("DetailTitle");

    QLabel* contentLbl = new QLabel(content);
    contentLbl->setObjectName("DetailContent");
    contentLbl->setWordWrap(true);
    if (title == QStringLiteral("角色描述")) {
        int maxLines = 2;
        int maxHeight = contentLbl->fontMetrics().lineSpacing() * maxLines + 4;
        contentLbl->setMaximumHeight(maxHeight);
        contentLbl->setToolTip(content);
    }
    if (!isEditable) {
        contentLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    contentLbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    layout->addWidget(titleLbl);
    layout->addWidget(contentLbl);

    if (isEditable) {
        QLabel* arrowLbl = new QLabel(">");
        arrowLbl->setStyleSheet(
            "color: #CCCCCC; font-weight: bold; font-family: consolas;");
        arrowLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        layout->addWidget(arrowLbl);
    }

    return item;
}

QFrame* ProfileWidget::createSeparatorFrame()
{
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setObjectName("DetailSeparator");
    line->setFixedHeight(1);
    return line;
}

void ProfileWidget::setAvatar(const QPixmap& pixmap)
{
    if (!pixmap.isNull()) {
        m_avatarLabel->setPixmap(buildRoundedAvatar(pixmap, 64));
    }
}

void ProfileWidget::setUserName(const QString& name)
{
    m_userNameRaw = name;
    updateHeaderLabels();
}

void ProfileWidget::setTmId(const QString& uuid)
{
    m_tmIdRaw = uuid;
    updateHeaderLabels();
}

void ProfileWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateHeaderLabels();
}

void ProfileWidget::updateHeaderLabels()
{
    if (!m_nameLabel || !m_idLabel) {
        return;
    }

    QString nameText = m_userNameRaw.isEmpty() ? m_nameLabel->text() : m_userNameRaw;
    QString idText = m_tmIdRaw.isEmpty() ? QStringLiteral("TmId: --")
                                         : QStringLiteral("TmId: ") + m_tmIdRaw;

    int nameWidth = m_nameLabel->width();
    int idWidth = m_idLabel->width();
    if (nameWidth > 0) {
        nameText = m_nameLabel->fontMetrics().elidedText(nameText, Qt::ElideRight, nameWidth);
    }
    if (idWidth > 0) {
        idText = m_idLabel->fontMetrics().elidedText(idText, Qt::ElideRight, idWidth);
    }

    m_nameLabel->setText(nameText);
    m_idLabel->setText(idText);
}

void ProfileWidget::setupStyle()
{
    QString qss = R"(
        ProfileWidget, #ContentWidget { background-color: #F6F7FA; }
        #HeaderWidget { background-color: #FFFFFF; }
        #AvatarLabel { border-radius: 32px; border: 1px solid #E5E5E5; }
        #NameLabel { font-family: "Microsoft YaHei"; font-size: 20px; font-weight: bold; color: #111111; }
        #IdLabel { font-family: "Microsoft YaHei"; font-size: 13px; color: #8A8A8A; }
        #MoreButton { border: none; background: transparent; font-size: 18px; color: #333333; font-weight: bold; }
        #MoreButton:hover { background-color: #F0F0F0; border-radius: 4px; }
        #GapFrame { background-color: #F6F7FA; border: none; }
        #DetailGroupTitle { color: #8A8A8A; font-size: 12px; padding-left: 20px; padding-top: 6px; padding-bottom: 6px; }
        #DetailGroup { background-color: #FFFFFF; border: 1px solid #ECECEC; border-radius: 8px; margin-left: 20px; margin-right: 20px; }
        #DetailTitle { font-size: 16px; color: #333333; }
        #DetailContent { font-size: 15px; color: #6F6F6F; line-height: 1.5; }
        #DetailSeparator { color: #EDEDED; }
        #BottomBar { background-color: #FFFFFF; border-top: 1px solid #E5E5E5; }
        #MsgButton { 
            background-color: #FFFFFF; border: 1px solid #E5E5E5; border-radius: 5px; 
            color: #576B95; font-size: 16px; font-weight: bold; 
        }
        #MsgButton:pressed { background-color: #F2F2F2; }
    )";
    this->setStyleSheet(qss);
}
