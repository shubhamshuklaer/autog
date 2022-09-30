#ifndef GRADER_MARKS_WIDGET_H
#define GRADER_MARKS_WIDGET_H

#include <QCheckBox>
#include <QList>
#include <QWidget>

namespace Ui {
class grader_marks_widget;
}

class grader_marks_widget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString marks READ get_marks WRITE put_marks)

public:
    explicit grader_marks_widget(QWidget *parent = 0,QStringList marks_denominations_list=QStringList());
    QString get_marks();
    void put_marks(QString marks);
    ~grader_marks_widget();

signals:
    void marks_changed();

private slots:
    void check_box_state_changed();
    void marks_edited();

private:
    Ui::grader_marks_widget *ui;
    QList<QCheckBox *> marks_check_boxes_list;
};

#endif // GRADER_MARKS_WIDGET_H
