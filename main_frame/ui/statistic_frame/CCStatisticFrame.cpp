#include "CCStatisticFrame.h"

#include <QLibrary>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include "CCTabWidget.h"

CCStatisticFrame::CCStatisticFrame(QWidget *parent) : QWidget(parent)
{
    setup_ui();
}

void CCStatisticFrame::setup_ui()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);
    CCTabWidget* tab = new CCTabWidget;

    QStringList allPlugins = {"./plugin/quality_record_plugin.dll"};
    foreach (const QString &path, allPlugins ) {
        QLibrary lib(path);
        if(lib.load())
        {
            qDebug()<<" load plugin:"<<path;
            using getPluginWidgetFunc = QWidget *(*)();
            getPluginWidgetFunc funcGetWindget = (getPluginWidgetFunc )lib.resolve("getPluginWidget");
            using getPluginTitleFunc = QString (*)();
            getPluginTitleFunc funcGetTitle = (getPluginTitleFunc )lib.resolve("getTitle");
            using getPluginIconFunc = QIcon (*)();
            getPluginIconFunc funcGetIcon = (getPluginIconFunc )lib.resolve("getIcon");
            if(!funcGetWindget || !funcGetTitle || !funcGetIcon )
            {
               qCritical()<<QString("加载库%1的函数失败!").arg(path);
            }
            QWidget *pWidget = funcGetWindget();
            QString title = funcGetTitle();
            QIcon icon = funcGetIcon();
            tab->addTab(pWidget,title,icon);
        }
    }

    this->setAutoFillBackground(true);
    this->setStyleSheet("QWidget{background-color: #F3F6FE;}");

    mainLayout->addWidget(tab);
    this->setLayout(mainLayout);
}
