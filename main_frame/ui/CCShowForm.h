#ifndef CCSHOWFORM_H
#define CCSHOWFORM_H

#include <QWidget>

namespace Ui {
class CCShowForm;
}

class CCShowForm : public QWidget
{
    Q_OBJECT

public:
    explicit CCShowForm(QWidget *parent = nullptr);
    ~CCShowForm();

private:
    Ui::CCShowForm *ui;
};

#endif // CCSHOWFORM_H
