#ifndef GRADER_EDITOR_H
#define GRADER_EDITOR_H

#include <QWidget>
#include <QMutex>
#include "grader_marks_widget.h"
#include "grader_file_sys.h"

namespace Ui {
class grader_editor;
}

class grader_editor : public QWidget
{
    Q_OBJECT

public:
    explicit grader_editor(QWidget *parent = 0,QString project_path=QString(),QString module_name=QString(),QStringList filesList=QStringList(),QStringList marks_denominations=QStringList());
    ~grader_editor();
    int current_index;
    int previous_comment_pos_index;
    QString project_path,module_name;
    QStringList filesList,marks_denominations;
    QString out_dir_name,sub_tex_name,tex_errors,main_tex_dir_name;
    grader_marks_widget *marks_widget;
    grader_file_sys *file_sys_interface;


private slots:
    void on_next_btn_clicked();

    void on_prev_btn_clicked();

    void on_preview_btn_clicked();

    void on_file_name_combo_activated(int index);

    void on_fix_file_btn_clicked();

    void on_see_errors_btn_clicked();

    void on_open_tex_btn_clicked();

private:
    Ui::grader_editor *ui;
    void setup_marks_widget(int index);



};

#endif // GRADER_EDITOR_H
