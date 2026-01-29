#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;
class QToolButton;
class QVBoxLayout;
class QFrame;
class QResizeEvent;

// ==========================================
// 内部辅助类：可点击的列表项
// ==========================================
class DetailItemWidget : public QWidget {
    Q_OBJECT
public:
    explicit DetailItemWidget(const QString& key, QWidget* parent = nullptr);
signals:
    void itemClicked(const QString& key);

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QString m_key;
};

// ==========================================
// 主组件类
// ==========================================
class ProfileWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget* parent = nullptr);

    // --- 基础信息接口 ---
    void setAvatar(const QPixmap& pixmap);
    void setUserName(const QString& name);
    void setTmId(const QString& uuid);

    // --- 详情列表接口 ---
    // isEditable: true 表示显示为可点击样式
    void addDetailItem(const QString& title, const QString& content, bool isEditable = false);
    void addSeparator();
    void clearDetails();

signals:
    void messageClicked();                        // 点击"发消息"
    void moreOptionsClicked();                    // 点击右上角"..."
    void detailItemClicked(const QString& title); // 点击列表项

private:
    void setupUi();
    void setupStyle();
    void updateHeaderLabels();

    // 内部辅助函数
    QFrame* createSeparatorFrame();
    QWidget* createDetailWidget(const QString& title, const QString& content, bool isEditable);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    // UI 控件
    QLabel* m_avatarLabel;
    QLabel* m_nameLabel;
    QLabel* m_idLabel;
    QPushButton* m_msgButton;
    QToolButton* m_moreButton;

    QVBoxLayout* m_detailListLayout;

    QString m_userNameRaw;
    QString m_tmIdRaw;
};

#endif // PROFILEWIDGET_H
