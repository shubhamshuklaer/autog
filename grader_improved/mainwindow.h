#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include "grader_editor.h"
#include "grader_project_load.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void setup_done();
private:
    Ui::MainWindow *ui;
    grader_editor *editor_widget;
    grader_project_load *project_load_widget;
    QString project_path,module_name;
    QStringList files_list;
    int start_grading_from;
};

#endif // MAINWINDOW_H
