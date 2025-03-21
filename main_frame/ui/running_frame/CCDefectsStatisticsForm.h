/**
  * @author ld
  * @brief  缺陷统计界面
  * @date   2022-11-28
  */
#ifndef CCDEFECTSSTATISTICSFORM_H
#define CCDEFECTSSTATISTICSFORM_H

#include <QFrame>
#include <QMap>
#include <QThread>

namespace Ui {
class DefectsStatisticsForm;
}

class CCDefectsStatisticsForm : public QFrame
{
    Q_OBJECT

public:
    explicit CCDefectsStatisticsForm(QWidget *parent = 0);
    ~CCDefectsStatisticsForm();
    /**
     * @brief clearTable 清除缺陷集合
     */
    void clearTable();
    /**
     * @brief addDefect 添加缺陷
     * @param blockID 块号
     * @param defect 缺陷名称
     */
    void addDefect(int blockID, QString defect);
private:
    QMap<int, QMap<QString, quint64>> m_defects;    // 缺陷集合 <块号 <缺陷名称, 数量>>
    Ui::DefectsStatisticsForm *ui;
};

#endif // CCDEFECTSSTATISTICSFORM_H
