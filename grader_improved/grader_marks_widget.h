#ifndef GRADER_MARKS_WIDGET_H
#define GRADER_MARKS_WIDGET_H

#include <QWidget>
#include <QList>
#include <QCheckBox>

namespace Ui {
class grader_marks_widget;
}

class grader_marks_widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString marks READ get_marks WRITE put_marks);

public:
    explicit grader_marks_widget(QWidget *parent = 0,QStringList marks_denominations=QStringList());
    QString get_marks();
    QList<QCheckBox *> check_box_list;
    void put_marks(QString marks);
    ~grader_marks_widget();
private slots:
    void marks_check_box_changed();

private:
    Ui::grader_marks_widget *ui;
signals:
    void marks_changed();
};

#endif // GRADER_MARKS_WIDGET_H
