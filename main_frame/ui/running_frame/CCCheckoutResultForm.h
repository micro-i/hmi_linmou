/**
  * @author ld
  * @brief  检出结果窗口
  * @date   2022-11-24
  */

#ifndef CCCHECKOUTRESULTFORM_H
#define CCCHECKOUTRESULTFORM_H

#include <QFrame>
#include <QVector>

namespace Ui {
class CheckoutResultForm;
}

class CCCheckoutResultForm : public QFrame
{
    Q_OBJECT

public:
    explicit CCCheckoutResultForm(QWidget *parent = 0);

    ~CCCheckoutResultForm();
    /**
     * @brief 拿到所有窗体
     * @return 窗体数组
     */
    QVector<class CCWorkpiece2d*> getAllFrames(){return m_workPieceFrames;}
    /**
     * @brief setScreenNum 设置屏幕数量
     * @param width 横向数量
     * @param height 纵向数量
     */
    void setScreenNum(const int width, const int height);
private:
    Ui::CheckoutResultForm *ui;
    QVector<class CCWorkpiece2d*> m_workPieceFrames; // 所有二维窗体指针
};

#endif // CCCHECKOUTRESULTFORM_H
