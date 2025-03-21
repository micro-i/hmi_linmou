#include "CCTabWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QEvent>
#include <QDebug>

CCTabWidget::CCTabWidget(QWidget *parent) : QWidget(parent)
{
    setup_ui();
}
void CCTabWidget::setup_ui()
{
    QWidget* leftWin = new QWidget;
    leftWin->setObjectName("leftWin");
    leftWin->setStyleSheet("QWidget#leftWin"
                           "{background-color:white;"
                           "background-image: url(:/images/plugin-background.png);"  /* 背景图片 */
                           "background-origin: content;"
                           "background-position: bottom;	"	/* 背景图片的位置 */
                           "padding-right: 2px;	"	    /* 背景图标的padding参数 */
                           "padding-bottom: 2px;"		/* 背景图标的padding参数 */
                           "background-repeat: no-repeat;"/* 设置背景图像的平铺模式 */
                           "}");
    m_leftLayout =new QVBoxLayout;
    m_leftLayout->setContentsMargins(0,20,0,20);
    m_leftLayout->addStretch();
    leftWin->setLayout(m_leftLayout);

    m_allTabs = new QStackedWidget;     // 所有的窗口

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(leftWin);
    mainLayout->addWidget(m_allTabs);
    mainLayout->setContentsMargins(0,0,0,0);
    this->setLayout(mainLayout);
}

void CCTabWidget::addTab(QWidget *tab,const QString &title,const QIcon &icon)
{
    QPushButton* btn = icon.isNull() \
                ? (new QPushButton(title))
                : (new QPushButton(icon,title));
    btn->setStyleSheet("QPushButton{"
                "border-left:0px;"
                "border-top-right-radius:10px;"
                "border-bottom-right-radius:10px;"
                "background-color:white;  "//背景颜色
                "color:black;             "//字体颜色
                "}"
                "QPushButton:checked { color:white; background-color:#385AFD;};"
                "QPushButton:!checked {color:black; background-color:#D9D9D9;};"
                );
    addTab(btn,tab);
}

void CCTabWidget::addTab(QPushButton* btn , QWidget* tab)
{
    btn->setCheckable(true);
    btn->setMinimumSize(200,60);
    m_leftLayout->insertWidget(m_leftLayout->count()-1,btn);

    int idx = m_allTabs->addWidget(tab);
    m_allTabs->setCurrentIndex(idx);
    btn->setChecked(true);

    auto onCheck = [=](){
        m_allTabs->setCurrentIndex(idx);
        // 取消上一次点击的按钮状态
        for(int i = m_leftLayout->count()-1;i >= 0; --i)
        {
            QPushButton* btn = dynamic_cast<QPushButton*>(m_leftLayout->itemAt(i)->widget());
            if(btn && btn->isChecked())
            {
                btn->setChecked(false);
            }
        }
        btn->setChecked(true);
    };
    onCheck();
    connect(btn,&QPushButton::clicked,onCheck);
}
