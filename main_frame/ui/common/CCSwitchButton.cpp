﻿#include "CCSwitchButton.h"
#include <QPainter>

CCSwitchButton::CCSwitchButton(QWidget *parent) : QWidget(parent)
{
    m_space = 2;
    m_radius = 5;
    m_checked = false;
    m_showText = true;
    m_showText = false;
    m_animation = true;

    m_bgColorOn = QColor(102, 205, 170);//设置按钮内部颜色
    m_bgColorOff = QColor(190, 190, 190);

    m_sliderColorOn = QColor(255, 255, 255);
    m_sliderColorOff = QColor(255, 255, 255);

    m_textColor = QColor(255, 255, 255);

    m_textOn = u8"普通用户";//设置按钮中的文字
    m_textOff = u8"管理员";

    m_step = 0;
    m_startX = 0;
    m_endX = 0;

    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateValue()));
}

void CCSwitchButton::drawBackGround(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor bgColor = m_checked ? m_bgColorOn : m_bgColorOff;
    if (isEnabled()) {
        bgColor.setAlpha(150);
    }

    painter->setBrush(bgColor);

    QRect rect(0, 0, width(), height());
    int side = qMin(width(), height());

    //左侧半圆
    QPainterPath path1;
    path1.addEllipse(rect.x(), rect.y(), side, side);

    //右侧半圆
    QPainterPath path2;
    path2.addEllipse(rect.width() - side, rect.y(), side, side);

    //中间的矩形
    QPainterPath path3;
    path3.addRect(rect.x() + side / 2, rect.y(), rect.width() - side, height());

    QPainterPath path = path1 + path2 + path3;
    painter->drawPath(path);

    //绘制文本

    //滑块半径
    int sliderWidth = qMin(height(), width()) - m_space * 2 - 5;
    if (m_checked){
        QRect textRect(0, 0, width() - sliderWidth, height());
        painter->setPen(QPen(m_textColor));
        painter->drawText(textRect, Qt::AlignCenter, m_textOn);
    } else {
        QRect textRect(sliderWidth, 0, width() - sliderWidth, height());
        painter->setPen(QPen(m_textColor));
        painter->drawText(textRect, Qt::AlignCenter, m_textOff);
    }

    painter->restore();
}

void CCSwitchButton::drawSlider(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor color = m_checked ? m_sliderColorOn : m_sliderColorOff;

    painter->setBrush(QBrush(color));

    int sliderWidth = qMin(width(), height()) - m_space * 2;
    QPainterPath path;

    QRect rect(m_space + m_startX, m_space, sliderWidth * 1.5, sliderWidth);
    path.addRoundedRect(rect, sliderWidth / 2, sliderWidth / 2);
    painter->drawPath(path);
    painter->restore();
}

void CCSwitchButton::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev)

    //启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    //绘制背景
    drawBackGround(&painter);

    //绘制滑块
    drawSlider(&painter);
}

void CCSwitchButton::mousePressEvent(QMouseEvent *ev)
{
    Q_UNUSED(ev)

    m_checked = !m_checked;
    emit statusChanged(m_checked);

    //计算步长
    m_step = width() / 10;

    //计算滑块X轴终点坐标
    if (m_checked) {
        m_endX = width() - height() * 1.5;
    } else {
        m_endX = 0;
    }

    //判断是否使用动画
    if (m_animation) {
        m_timer->start();
    } else{
        m_startX = m_endX;
        update();
    }
}

void CCSwitchButton::updateValue()
{
    if (m_checked) {
        if (m_startX < m_endX) {
            m_startX += m_step;
        } else {
            m_startX = m_endX;
            m_timer->stop();
        }
    } else {
        if (m_startX > m_endX) {
            m_startX -= m_step;
        } else {
            m_startX = m_endX;
            m_timer->stop();
        }
    }

    update();
}

int CCSwitchButton::space() const
{
    return m_space;
}

int CCSwitchButton::radius() const
{
    return m_radius;
}

bool CCSwitchButton::checked() const
{
    return m_checked;
}

bool CCSwitchButton::showText() const
{
    return m_showText;
}

bool CCSwitchButton::showCircel() const
{
    return m_showCircle;
}

bool CCSwitchButton::animation() const
{
    return m_animation;
}

QColor CCSwitchButton::bgColorOn() const
{
    return m_bgColorOn;
}

QColor CCSwitchButton::bgColorOff() const
{
    return m_bgColorOff;
}

QColor CCSwitchButton::sliderColorOn() const
{
    return m_sliderColorOn;
}

QColor CCSwitchButton::sliderColorOff() const
{
    return m_sliderColorOff;
}

QColor CCSwitchButton::textColor() const
{
    return m_textColor;
}

QString CCSwitchButton::textOn() const
{
    return m_textOn;
}

QString CCSwitchButton::textOff() const
{
    return m_textOff;
}

int CCSwitchButton::step() const
{
    return m_step;
}

int CCSwitchButton::startX() const
{
    return m_startX;
}

int CCSwitchButton::endX() const
{
    return m_endX;
}

void CCSwitchButton::setSpace(int space)
{
    if (m_space != space) {
        m_space = space;
        update();
    }
}

void CCSwitchButton::setRadius(int radius)
{
    if (m_radius != radius) {
        m_radius = radius;
        update();
    }
}

void CCSwitchButton::setChecked(bool checked)
{
    if (m_checked != checked) {
        m_checked = checked;
        update();
    }
}

void CCSwitchButton::setShowText(bool show)
{
    if (m_showText != show) {
        m_showText = show;
        update();
    }
}

void CCSwitchButton::setShowCircle(bool show)
{
    if (m_showCircle != show) {
        m_showCircle = show;
        update();
    }
}

void CCSwitchButton::setAnimation(bool ok)
{
    if (m_animation != ok) {
        m_animation = ok;
        update();
    }
}

void CCSwitchButton::setBgColorOn(const QColor &color)
{
    if (m_bgColorOn != color) {
        m_bgColorOn = color;
        update();
    }
}

void CCSwitchButton::setBgColorOff(const QColor &color)
{
    if (m_bgColorOff != color) {
        m_bgColorOff = color;
        update();
    }
}

void CCSwitchButton::setSliderColorOn(const QColor &color)
{
    if (m_sliderColorOn != color) {
        m_sliderColorOn = color;
        update();
    }
}

void CCSwitchButton::setSliderColorOff(const QColor &color)
{
    if (m_sliderColorOff != color) {
        m_sliderColorOff = color;
        update();
    }
}

void CCSwitchButton::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        update();
    }
}

void CCSwitchButton::setTextOn(const QString &text)
{
    if (m_textOn != text) {
        m_textOn = text;
        update();
    }
}

void CCSwitchButton::setTextOff(const QString &text)
{
    if (m_textOff != text) {
        m_textOff = text;
        update();
    }
}
