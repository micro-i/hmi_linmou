/**
  * @author ld
  * @brief  带有圆角的对话框
  * @date   2023-02-14
  */

#ifndef CCRADIUSDIALOG_H
#define CCRADIUSDIALOG_H

#include <QDialog>

class CCRadiusDialog : public QDialog
{
    Q_OBJECT

enum WINDOWPOSITION{
    NONE,
    LEFTTOP,
    LEFTBOTTOM,
    RIGHTTOP,
    RIGHTBOTTOM,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

public:
    explicit CCRadiusDialog(QWidget *parent = nullptr);
    virtual ~CCRadiusDialog() override;
    // 设置圆角半径
    void setRadius(int radius = 5);
    void setWindowDrag(bool drag);
protected:
    /**
     * @brief paintEvent: draw border radius
     */
    void paintEvent(QPaintEvent *) override;
    void region(const QPoint &currentGlobalPoint);
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
protected:
    int             m_radius = 5;               // border radius
    int             m_borderWidth = 0;          // border width
    int             m_padding = 10;             // padding
    bool            m_isLeftPressDown=false;    // left mouse down flag
    bool            m_windowDrag = false;       // left mouse move flag
    QPoint          m_movePoint;                // left mouse down point
    WINDOWPOSITION  m_pos;                      // change cursor status with different pos

};

#endif // CCRADIUSDIALOG_H
