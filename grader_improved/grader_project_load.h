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
    Q_PROPERTY(QString module_name READ get_module_name);
    Q_PROPERTY(QString project_path READ get_project_path);
    Q_PROPERTY(QStringList filesList READ get_filesList);
    Q_PROPERTY(QStringList marks_denominations READ get_marks_denominations);
public:
    explicit grader_project_load(QWidget *parent = 0);
    QString get_module_name();
    QString get_project_path();
    QStringList get_filesList();
    QStringList get_marks_denominations();
    ~grader_project_load();
signals:
    void done();
private slots:
    void on_select_project_btn_clicked();

    void on_start_btn_clicked();

private:
    Ui::grader_project_load *ui;
    QString module_name,project_path;
    QStringList filesList,marks_denominations;
    bool parse_project_config(QString);
    bool setup_module();
    void load_settings();
};

#endif // GRADER_LOAD_H
