#ifndef GRADER_EDITOR_H
#define GRADER_EDITOR_H

#include <QWidget>
#include <QMutex>

namespace Ui {
class grader_editor;
}

class grader_editor : public QWidget
{
    Q_OBJECT

public:
    explicit grader_editor(QWidget *parent = 0,QStringList filesList=QStringList(),QString out_dir_name=QString());
    ~grader_editor();
    int current_index;
    QStringList filesList;
    QString out_dir_name;
    QMutex file_mutex,tex_mutex,main_file_mutex;
    QString get_marks(QString file_name);
    QString get_comment(QString file_name);
    void put_marks(QString file_name , QString marks);
    void put_comment(QString file_name , QString comment);
    void include_only(bool);

private slots:
    void on_next_btn_clicked();

    void on_prev_btn_clicked();

    void on_preview_btn_clicked();

    void on_gen_pdf_btn_clicked();

    void on_file_name_combo_activated(int index);

private:
    Ui::grader_editor *ui;
};

#endif // GRADER_EDITOR_H
