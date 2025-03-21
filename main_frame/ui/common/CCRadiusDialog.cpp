#include "CCRadiusDialog.h"
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>

#include "infrastructure/CCSignalBridge.h"  // set mask

CCRadiusDialog::CCRadiusDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setMouseTracking(true);
}

CCRadiusDialog::~CCRadiusDialog()
{

}

void CCRadiusDialog::setRadius(int radius)
{
    m_radius = radius;
}

void CCRadiusDialog::setWindowDrag(bool drag)
{
    m_windowDrag = drag;
}

void CCRadiusDialog::region(const QPoint &currentGlobalPoint)
{
    // 获取窗体在屏幕上的位置区域，topLeft为坐上角点，rightButton为右下角点
    QRect rect = this->rect();

    QPoint topLeft = this->mapToGlobal(rect.topLeft()); //将左上角的(0,0)转化为全局坐标
    QPoint rightBottom = this->mapToGlobal(rect.bottomRight());

    int x = currentGlobalPoint.x(); //当前鼠标的坐标
    int y = currentGlobalPoint.y();

    if(((topLeft.x() + m_padding >= x) && (topLeft.x() <= x))
            && ((topLeft.y() + m_padding >= y) && (topLeft.y() <= y)))
    {
        // 左上角
        m_pos = LEFTTOP;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));  // 设置光标形状
    }else if(((x >= rightBottom.x() - m_padding) && (x <= rightBottom.x()))
              && ((y >= rightBottom.y() - m_padding) && (y <= rightBottom.y())))
    {
        // 右下角
        m_pos = RIGHTBOTTOM;
        this->setCursor(QCursor(Qt::SizeFDiagCursor));
    }else if(((x <= topLeft.x() + m_padding) && (x >= topLeft.x()))
              && ((y >= rightBottom.y() - m_padding) && (y <= rightBottom.y())))
    {
        //左下角
        m_pos = LEFTBOTTOM;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    }else if(((x <= rightBottom.x()) && (x >= rightBottom.x() - m_padding))
              && ((y >= topLeft.y()) && (y <= topLeft.y() + m_padding)))
    {
        // 右上角
        m_pos = RIGHTTOP;
        this->setCursor(QCursor(Qt::SizeBDiagCursor));
    }else if((x <= topLeft.x() + m_padding) && (x >= topLeft.x()))
    {
        // 左边
        m_pos = LEFT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if((x <= rightBottom.x()) && (x >= rightBottom.x() - m_padding))
    {
        // 右边
        m_pos = RIGHT;
        this->setCursor(QCursor(Qt::SizeHorCursor));
    }else if((y >= topLeft.y()) && (y <= topLeft.y() + m_padding))
    {
        // 上边
        m_pos = UP;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else if((y <= rightBottom.y()) && (y >= rightBottom.y() - m_padding))
    {
        // 下边
        m_pos = DOWN;
        this->setCursor(QCursor(Qt::SizeVerCursor));
    }else
    {
        // 默认
        m_pos = NONE;
        this->setCursor(QCursor(Qt::ArrowCursor));
    }
}

void CCRadiusDialog::closeEvent(QCloseEvent *e)
{
    emit CCSignalBridge::instance()->setMask(false);
    QDialog::closeEvent(e);
}

void CCRadiusDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape){
        return;
    }
    QDialog::keyPressEvent(e);
}

//鼠标按下事件
void CCRadiusDialog::mousePressEvent(QMouseEvent *event)
{
    if(!m_windowDrag){
        QDialog::mousePressEvent(event);
        return;
    }
    switch(event->button())
    {
        case Qt::LeftButton:
            m_isLeftPressDown = true;

            if(m_pos != NONE)
            {
                this->mouseGrabber(); //返回当前抓取鼠标输入的窗口
            }
            else
            {
                m_movePoint = event->globalPos() - this->frameGeometry().topLeft();
            }
            break;
        case Qt::RightButton:
            this->setWindowState(Qt::WindowMinimized);
            break;
        default:
            QDialog::mousePressEvent(event);
    }
}

//鼠标移动事件
void CCRadiusDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_windowDrag){
        QDialog::mouseMoveEvent(event);
        return;
    }
    QPoint globalPoint = event->globalPos();   //鼠标全局坐标

    QRect rect = this->rect();  //rect == QRect(0,0 1280x720)
    QPoint topLeft = mapToGlobal(rect.topLeft());
    QPoint bottomRight = mapToGlobal(rect.bottomRight());

    if (this->windowState() != Qt::WindowMaximized)
    {
        if(!m_isLeftPressDown)  //没有按下左键时
        {
            this->region(globalPoint); //窗口大小的改变——判断鼠标位置，改变光标形状
        }
        else
        {
            if(m_pos != NONE)
            {
                QRect newRect(topLeft, bottomRight); //定义一个矩形
                switch(m_pos)
                {
                    case LEFT:

                        if(bottomRight.x() - globalPoint.x() <= this->minimumWidth())
                        {
                            newRect.setLeft(topLeft.x());  //小于界面的最小宽度时，设置为左上角横坐标为窗口x
                            //只改变左边界
                        }
                        else
                        {
                            newRect.setLeft(globalPoint.x());
                        }
                        break;
                    case RIGHT:
                        newRect.setWidth(globalPoint.x() - topLeft.x());  //只能改变右边界
                        break;
                    case UP:
                        if(bottomRight.y() - globalPoint.y() <= this->minimumHeight())
                        {
                            newRect.setY(topLeft.y());
                        }
                        else
                        {
                            newRect.setY(globalPoint.y());
                        }
                        break;
                    case DOWN:
                        newRect.setHeight(globalPoint.y() - topLeft.y());
                        break;
                    case LEFTTOP:
                        if(bottomRight.x() - globalPoint.x() <= this->minimumWidth())
                        {
                            newRect.setX(topLeft.x());
                        }
                        else
                        {
                            newRect.setX(globalPoint.x());
                        }

                        if(bottomRight.y() - globalPoint.y() <= this->minimumHeight())
                        {
                            newRect.setY(topLeft.y());
                        }
                        else
                        {
                            newRect.setY(globalPoint.y());
                        }
                        break;
                     case RIGHTTOP:
                          if (globalPoint.x() - topLeft.x() >= this->minimumWidth())
                          {
                              newRect.setWidth(globalPoint.x() - topLeft.x());
                          }
                          else
                          {
                              newRect.setWidth(bottomRight.x() - topLeft.x());
                          }
                          if (bottomRight.y() - globalPoint.y() >= this->minimumHeight())
                          {
                              newRect.setY(globalPoint.y());
                          }
                          else
                          {
                              newRect.setY(topLeft.y());
                          }
                          break;
                     case LEFTBOTTOM:
                          if (bottomRight.x() - globalPoint.x() >= this->minimumWidth())
                          {
                              newRect.setX(globalPoint.x());
                          }
                          else
                          {
                              newRect.setX(topLeft.x());
                          }
                          if (globalPoint.y() - topLeft.y() >= this->minimumHeight())
                          {
                              newRect.setHeight(globalPoint.y() - topLeft.y());
                          }
                          else
                          {
                              newRect.setHeight(bottomRight.y() - topLeft.y());
                          }
                          break;
                      case RIGHTBOTTOM:
                          newRect.setWidth(globalPoint.x() - topLeft.x());
                          newRect.setHeight(globalPoint.y() - topLeft.y());
                          break;
                      default:
                          break;
                }
                this->setGeometry(newRect);
            }
        }
    }
}


//鼠标释放事件
void CCRadiusDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isLeftPressDown = false;
        if (m_pos != NONE)
        {
            this->releaseMouse(); //释放鼠标抓取
            this->setCursor(QCursor(Qt::ArrowCursor));
            m_pos = NONE; //热心网友指正
        }

    }
}




void CCRadiusDialog::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    if (!(windowFlags() & Qt::FramelessWindowHint) && !testAttribute(Qt::WA_TranslucentBackground))
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF rect(QPointF(0, 0), size());
    qreal penWidth = m_borderWidth;
    if (penWidth < 0.0) {
        QStyleOption opt;
        opt.initFrom(this);
        penWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this);
    }
    if (penWidth > 0.0) {
        p.setPen(QPen(palette().brush(foregroundRole()), penWidth));
        const qreal dlta = penWidth * 0.5;
        rect.adjust(dlta, dlta, -dlta, -dlta);
    }
    else {
        p.setPen(Qt::NoPen);
    }

    p.setBrush(palette().brush(backgroundRole()));
    if (m_radius > 0.0)
        p.drawRoundedRect(rect, m_radius, m_radius, Qt::AbsoluteSize);
    else
        p.drawRect(rect);

    p.end();
}
