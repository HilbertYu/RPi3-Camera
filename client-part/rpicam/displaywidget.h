#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QWidget>

namespace Ui {
class displayWidget;
}

class displayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit displayWidget(QWidget *parent = 0);
    ~displayWidget();

private:
    Ui::displayWidget *ui;


public slots:
    void run(void);
};

#endif // DISPLAYWIDGET_H
