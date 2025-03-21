/**
  * @author ld
  * @brief   公共弹窗标题栏控件
  * @date    2022-11-24
  */

#ifndef PUBLICTILEBAR_H
#define PUBLICTILEBAR_H

#include <QWidget>

namespace Ui {
class CCPublicTitleBar;
}

class CCPublicTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit CCPublicTitleBar(QWidget *parent = nullptr);
    ~CCPublicTitleBar() override;
    // 设置标题
    void setTitle(QString title);
private:
    // 标题点击关闭事件与拖拽移动事件
    bool eventFilter(QObject *obj, QEvent *event) override;
    Ui::CCPublicTitleBar *ui;
};

#endif // PUBLICTILEBAR_H
