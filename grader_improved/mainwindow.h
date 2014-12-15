#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    grader_editor *grader_editor_widget;
    grader_project_load *grader_load_widget;
    QString project_path,module_name;
    QStringList filesList,marks_denominations;

    ~MainWindow();

private slots:
    void setup_done();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
