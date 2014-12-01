#ifndef GRADER_EDITOR_H
#define GRADER_EDITOR_H

#include <QWidget>

namespace Ui {
class grader_editor;
}

class grader_editor : public QWidget
{
    Q_OBJECT

public:
    explicit grader_editor(QWidget *parent = 0,QStringList filesList=QStringList());
    ~grader_editor();

private:
    Ui::grader_editor *ui;
};

#endif // GRADER_EDITOR_H
