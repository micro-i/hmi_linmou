#ifndef CSTATISTICFRAME_H
#define CSTATISTICFRAME_H

#include <QWidget>

class CCStatisticFrame : public QWidget
{
    Q_OBJECT
public:
    explicit CCStatisticFrame(QWidget *parent = nullptr);

private:
    void setup_ui();
signals:

public slots:
};

#endif // CSTATISTICFRAME_H
