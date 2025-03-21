#ifndef CCCONTROLPANEL_H
#define CCCONTROLPANEL_H

#include <QWidget>

// TODO 待启用
namespace Ui {
class CCControlPanel;
}

class CCControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CCControlPanel(QWidget *parent = nullptr);
    ~CCControlPanel();
    void setMaxIndex(int maxIndex);
private slots:
    void slot_enableStateChanged(int arg1);
    void slot_disable3D(int state);
    void slot_directionChanged(int idx);
    void slot_frameSliderMoved(int position);
signals:
    void enableCutter(bool);
    void directionChanged(int);
    void frameChanged(int);
    void disable3D(int);
private:
    Ui::CCControlPanel *ui;
};

#endif // CCCONTROLPANEL_H
