#ifndef GRADER_FILE_SYS_H
#define GRADER_FILE_SYS_H

#include <QObject>
#include <QMutex>

class grader_file_sys : public QObject
{
    Q_OBJECT
public:
    explicit grader_file_sys(QObject *parent = 0,QString main_tex_dir_name=QString(),QString out_dir_name=QString());
    QString get_marks(QString file_name);
    QString get_comment(QString file_name,QString comment_pos);
    Q_INVOKABLE void put_marks(QString file_name , QString marks);
    Q_INVOKABLE void put_comment(QString file_name , QString comment,QString comment_pos);
    Q_INVOKABLE QString generate_pdf(QString file_name,QString marks,QString comment_text,QString comment_pos);

    QString main_tex_dir_name,out_dir_name;

signals:
    void send_error(QString error);

public slots:

private:
    QMutex file_mutex,tex_mutex,main_file_mutex;
    void include_only(bool is_include_only,QString file_name);
    QString escape_string(QString comment);

};

#endif // GRADER_FILE_SYS_H