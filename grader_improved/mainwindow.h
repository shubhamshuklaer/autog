#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "grader_settings.h"
#include "grader_editor.h"
#include "grader_setup.h"
#include "grader_project_load.h"

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
    grader_project_load *grader_load_widget;
    QString project_path,module_name;

    ~MainWindow();

private slots:
    void open_config_window();
    void setup_done();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
