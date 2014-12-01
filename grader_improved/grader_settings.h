#ifndef GRADER_SETTINGS_H
#define GRADER_SETTINGS_H

#include <QWidget>

namespace Ui {
class grader_settings;
}

class grader_settings : public QWidget
{
    Q_OBJECT

public:
    explicit grader_settings(QWidget *parent = 0);
    ~grader_settings();

private slots:
    void on_load_default_settings_clicked();

    void on_save_settings_btn_clicked();

    void on_cancel_settings_btn_clicked();

private:
    Ui::grader_settings *ui;
};

#endif // GRADER_SETTINGS_H
