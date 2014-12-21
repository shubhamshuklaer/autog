#ifndef GRADER_LOAD_H
#define GRADER_LOAD_H

#include <QWidget>
#include <QDir>

namespace Ui {
class grader_project_load;
}

class grader_project_load : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList files_list READ get_files_list)
    Q_PROPERTY(QStringList marks_denominations_list READ get_marks_denominations_list)
    Q_PROPERTY(QString module_name READ get_module_name)
    Q_PROPERTY(QString project_path READ get_project_path)

public:
    explicit grader_project_load(QWidget *parent = 0);
    QString get_module_name();
    QString get_project_path();
    QStringList get_files_list();
    QStringList get_marks_denominations_list();
    ~grader_project_load();

signals:
    void done();

private slots:
    void select_project_btn_clicked();
    void start_btn_clicked();

private:
    Ui::grader_project_load *ui;
    QString module_name,project_path;
    QStringList files_list,marks_denominations_list;

    bool configure_project(QString);
    bool setup_module();
    void display_error(QString);
    void load_settings();

};

#endif // GRADER_LOAD_H
