/**
  * @author shenhuaibin
  * @brief  通用窗口切换Tab控件
  * @date   2022-12-05
*/
#ifndef CTABWIDGET_H
#define CTABWIDGET_H

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QWidget>
#include <QMap>

class CCTabWidget : public QWidget
{
    Q_OBJECT
public:
    CCTabWidget(QWidget *parent = nullptr);
    /**
     * @brief addTab 添加一个窗口
     * @param title 标题
     * @param widget 窗口实例
     * @param icon 图标
     */
    void addTab(QWidget *tab,const QString &title,const QIcon &icon);

protected:

private:
    /**
     * @brief addTab 内部方法.执行添加窗口
     * @param btn 左侧按钮
     * @param tab 右侧对应窗口
     */
    void addTab(QPushButton* btn , QWidget* tab);
    /**
     * @brief setup_ui 创建控件布局
     */
    void setup_ui();
signals:

private:
    QVBoxLayout* 				m_leftLayout ;
    QStackedWidget*             m_allTabs;     // 所有的窗口
};

#endif // CTABWIDGET_H
