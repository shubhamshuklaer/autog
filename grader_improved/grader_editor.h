#ifndef GRADER_EDITOR_H
#define GRADER_EDITOR_H

#include <QWidget>
#include <QMutex>
#include "grader_marks_widget.h"
#include "grader_file_sys.h"
#include <QThread>
#include <QList>
#include <QComboBox>
#include <QLabel>

namespace Ui {
class grader_editor;
}

class grader_editor : public QWidget
{
    Q_OBJECT

public:
    explicit grader_editor(QWidget *parent = 0,QString project_path=QString(),QString module_name=QString(),QStringList files_list=QStringList(),QStringList marks_denominations_list=QStringList(),QList<QStringList> merge_list=QList<QStringList>(),int start_grading_from=0);
    ~grader_editor();
    int current_index,current_merge_index;
    int previous_comment_pos_index;
    QString project_path,module_name,module_dir_path,tex_compile_errors;
    QStringList files_list,marks_denominations_list;
    QList<QStringList> merge_list;
    grader_marks_widget *marks_widget;
    grader_file_sys *file_sys_interface;
    QThread *file_sys_thread;
    QMutex tex_compile_errors_lock;
    QComboBox *merge_combo_box;
    QLabel *merge_label;
    void put_comment(bool async,QString file_name , QString comment,QString comment_pos);
    void put_marks( bool async, QString file_name ,QString marks );
    void generate_pdf(bool async,QString file_name,QString marks,QString comment_text,QString comment_pos);

private slots:
    void on_next_btn_clicked();

    void on_prev_btn_clicked();

    void on_preview_btn_clicked();

    void on_file_name_combo_activated(int index);

    void on_see_errors_btn_clicked();

    void on_open_tex_btn_clicked();

    void on_marks_text_textChanged();

    void on_comment_text_textChanged();

    void on_comment_pos_combo_activated(int index);

    void on_merge_combo_activated(int index);

    void set_tex_compile_error(QString error);

public slots:
    void display_errors_slot(QString);
private:
    Ui::grader_editor *ui;
    void setup_marks_widget(int index);
    void setup_merge_widget(int index);
    int get_merge_index();
    void display_error(QString);



};

#endif // GRADER_EDITOR_H
