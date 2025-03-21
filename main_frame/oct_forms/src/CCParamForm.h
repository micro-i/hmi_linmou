#ifndef CCPARAMFORM_H
#define CCPARAMFORM_H

#include <QWidget>

namespace Ui {
class CCParamForm;
}

class CCParamForm : public QWidget
{
    Q_OBJECT

public:
    explicit CCParamForm(QWidget *parent = nullptr);
    ~CCParamForm();

private:
    Ui::CCParamForm *ui;
};

#endif // CCPARAMFORM_H
