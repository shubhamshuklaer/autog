#ifndef GRADER_SETUP_H
#define GRADER_SETUP_H

#include <QWidget>
#include <QDir>

namespace Ui {
class grader_setup;
}

class grader_setup : public QWidget
{
    Q_OBJECT

public:
    explicit grader_setup(QWidget *parent = 0);
    QString burst_dir_name,top_tex_name,out_dir_name,sub_tex_name;
    QDir burst_dir;
    QStringList filesList;
    ~grader_setup();
signals:
    void done();
private slots:
    void choose_folder(void);
    void choose_top(void);
    void choose_subfile(void);
    void choose_out_folder(void);
    void setup_output(void);

    void on_start_new_btn_clicked();

private:
    Ui::grader_setup *ui;
};

#endif // GRADER_SETUP_H
