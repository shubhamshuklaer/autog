#ifndef GRADER_LOAD_H
#define GRADER_LOAD_H

#include <QWidget>
#include <QDir>

namespace Ui {
class grader_load;
}

class grader_project_load : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString module_name READ get_module_name);
    Q_PROPERTY(QString project_path READ get_project_path);
public:
    explicit grader_project_load(QWidget *parent = 0);
    QString get_module_name();
    QString get_project_path();
    ~grader_project_load();
signals:
    void done();
private slots:
    void on_select_project_btn_clicked();

    void on_start_btn_clicked();

private:
    Ui::grader_load *ui;
    QString module_name,project_path;
    QDir project_dir;
    bool parse_project_config(QString);
};

#endif // GRADER_LOAD_H
