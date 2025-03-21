/**
  * @author ld
  * @brief  位置示意图页面 2022-11-29更改为三维显示
  * @date   2022-11-28
  */

#ifndef CCPOSITIONVIEWFORM_H
#define CCPOSITIONVIEWFORM_H

#include <QFrame>
#include <ui/workpiece_frame/CCWorkpiece3d.h>

namespace Ui {
class PositionViewForm;
}

class CCPositionViewForm : public QFrame
{
    Q_OBJECT

public:
    explicit CCPositionViewForm(QWidget *parent = 0);
    ~CCPositionViewForm();
    /**
     * @brief getView3d 获取3d工件widget
     * @return
     */
    CCWorkpiece3d* getView3d();

private:
    Ui::PositionViewForm *ui;
};

#endif // CCPOSITIONVIEWFORM_H
