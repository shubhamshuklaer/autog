#ifndef GRADER_FILE_SYS_H
#define GRADER_FILE_SYS_H

#include <QObject>
#include <QMutex>

class grader_file_sys : public QObject
{
    Q_OBJECT
public:
    explicit grader_file_sys(QObject *parent = 0,QString module_dir_path=QString());
    QString get_marks(QString file_name);
    QString get_comment(QString file_name,QString comment_pos);
    void fix_file(QString file_name,QString marks_denominations);
    Q_INVOKABLE void put_marks(QString file_name , QString marks);
    Q_INVOKABLE void put_comment(QString file_name , QString comment,QString comment_pos);
    Q_INVOKABLE QString generate_pdf(QString file_name,QString marks,QString comment_text,QString comment_pos);



signals:
    void send_tex_compile_error(QString);
    void send_error(QString);

public slots:

private:
    QMutex sub_tex_files_edit_lock,tex_compile_lock,main_tex_edit_lock;
    QString module_dir_path;
};

#endif // GRADER_FILE_SYS_H
