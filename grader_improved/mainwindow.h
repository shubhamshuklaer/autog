#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "grader_settings.h"
#include "grader_editor.h"
#include "grader_setup.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    grader_settings *grader_settings_widget;
    grader_editor *grader_editor_widget;
    grader_setup *grader_setup_widget;
    QString out_dir_name;
    QStringList filesList;

    ~MainWindow();

private slots:
    void open_config_window();
    void setup_done();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
