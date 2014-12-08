#ifndef GRADER_EDITOR_H
#define GRADER_EDITOR_H

#include <QWidget>
#include <QMutex>
#include <QtConcurrent/QtConcurrent>
#include <QReadWriteLock>
#include "grader_marks_widget.h"

namespace Ui {
class grader_editor;
}

class grader_editor : public QWidget
{
    Q_OBJECT

public:
    explicit grader_editor(QWidget *parent = 0,QString project_path=QString(),QString module_name=QString());
    ~grader_editor();
    int current_index;
    QString project_path,module_name;
    QStringList filesList,marks_denominations;
    QString out_dir_name,sub_tex_name,tex_errors;
    QMutex file_mutex,tex_mutex,main_file_mutex;
    QReadWriteLock tex_errors_lock;
    grader_marks_widget *marks_widget;
    QFuture<void> future;
    QString get_marks(QString file_name);
    QString get_comment(QString file_name);
    void put_marks(QString file_name , QString marks);
    void put_comment(QString file_name , QString comment);
    void put_comment_pos(QString file_name, QString comment_pos);
    void include_only(bool);
    void preview_thread_func_marks(void);
    void preview_thread_func_comment(void);
    void preview_thread_func_comment_pos(QString comment_pos);
    QString escape_string(QString comment);


private slots:
    void on_next_btn_clicked();

    void on_prev_btn_clicked();

    void on_preview_btn_clicked();

    void on_gen_pdf_btn_clicked();

    void on_file_name_combo_activated(int index);

    void on_marks_text_textChanged();

    void on_comment_text_textChanged();

    void on_comment_pos_combo_activated(int index);

    void on_fix_file_btn_clicked();

    void on_see_errors_btn_clicked();

private:
    Ui::grader_editor *ui;
};

#endif // GRADER_EDITOR_H
