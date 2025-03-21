#include "CCDefectsStatisticsForm.h"
#include "ui_CCDefectsStatisticsForm.h"

#include <QStyleFactory>

CCDefectsStatisticsForm::CCDefectsStatisticsForm(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DefectsStatisticsForm)
{
    ui->setupUi(this);
    ui->tableDefects->setFocusPolicy(Qt::NoFocus);
    ui->tableDefects->setSelectionMode(QAbstractItemView::NoSelection);
    this->setStyle(QStyleFactory::create("windows"));
}

CCDefectsStatisticsForm::~CCDefectsStatisticsForm()
{
    delete ui;
}

void CCDefectsStatisticsForm::clearTable()
{
    m_defects.clear();
}

void CCDefectsStatisticsForm::addDefect(int blockID, QString defect)
{
    // 清空表格
    ui->tableDefects->clearContents();
    m_defects[blockID][defect]++;
    // 获取单元格
    static auto getItem = [](QString text){
      auto item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);
      return item;
    };

    auto temp = 0;
    for(auto it = m_defects.begin(); it != m_defects.end(); it++){
        for(auto it2 = it.value().begin(); it2 != it.value().end(); it2++){
            // 防止内存泄露
            auto item0 = ui->tableDefects->item(temp, 0);
            auto item1 = ui->tableDefects->item(temp, 1);
            auto item2 = ui->tableDefects->item(temp, 2);

            ui->tableDefects->setItem(temp, 0, nullptr == item0 ? getItem(QString::number(it.key())) : item0);
            ui->tableDefects->setItem(temp, 1, nullptr == item1 ? getItem(it2.key()) : item1);
            ui->tableDefects->setItem(temp, 2, nullptr == item2 ? getItem(QString::number(it2.value())) : item2);

            temp++;
        }
    }
}
