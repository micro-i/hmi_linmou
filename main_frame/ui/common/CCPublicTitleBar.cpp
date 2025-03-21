#include "CCPublicTitleBar.h"
#include "ui_CCPublicTitleBar.h"
#include <QMouseEvent>

CCPublicTitleBar::CCPublicTitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CCPublicTitleBar)
{
    ui->setupUi(this);
    ui->closeLabel->installEventFilter(this);
    ui->titleLabel->installEventFilter(this);
}

CCPublicTitleBar::~CCPublicTitleBar()
{
    delete ui;
}

void CCPublicTitleBar::setTitle(QString title)
{
    ui->titleLabel->setText(title);
}

bool CCPublicTitleBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->closeLabel && event)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if(mouseEvent && mouseEvent->button() == Qt::LeftButton && this->parent())
            {
                try {
                    (static_cast<QWidget*>(this->parent())->close());
                } catch (...) {
                    return false;
                }
                return true;
            }
        }
    }
    else if(obj == ui->titleLabel && event){
        static bool isPressed = false;
        static QPoint m_pressedPoint;

        if(event->type() == QEvent::MouseMove){
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if(mouseEvent && isPressed && this->parent()){
                try {
                    QPoint parentPoint = (static_cast<QWidget*>(this->parent())->pos());
                    parentPoint.setX(parentPoint.x() + mouseEvent->x() - m_pressedPoint.x());
                    parentPoint.setY(parentPoint.y() + mouseEvent->y() - m_pressedPoint.y());
                    (static_cast<QWidget*>(this->parent())->move(parentPoint));
                } catch (...) {
                    return false;
                }
                return true;
            }
        }else if(event->type() == QEvent::MouseButtonPress){
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if(mouseEvent && mouseEvent->button() == Qt::LeftButton){
                isPressed = true;
                m_pressedPoint = mouseEvent->pos();
                return true;
            }
        }else if(event->type() == QEvent::MouseButtonRelease){
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
            if(mouseEvent && mouseEvent->button() == Qt::LeftButton){
                isPressed = false;
                return true;
            }
        }
    }
    return false;
}
