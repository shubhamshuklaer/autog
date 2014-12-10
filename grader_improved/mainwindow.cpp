#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "grader_editor.h"
#include "grader_settings.h"
#include "grader_project_load.h"
#include <QDebug>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->grader_settings_widget=new grader_settings();
    this->grader_load_widget=new grader_project_load(this);
    this->ui->centralWidget->setFixedSize(this->grader_load_widget->size());
    connect( this->ui->config, SIGNAL(triggered()), this, SLOT(open_config_window()));
    connect( this->grader_load_widget, SIGNAL(done()),this,SLOT(setup_done()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::open_config_window(){
    grader_settings_widget->show();
}

void MainWindow::setup_done(){
    this->project_path=this->grader_load_widget->property("project_path").toString();
    this->module_name=this->grader_load_widget->property("module_name").toString();
    this->filesList=this->grader_load_widget->property("filesList").toStringList();
    this->marks_denominations=this->grader_load_widget->property("marks_denominations").toStringList();
    delete this->grader_load_widget;
    this->setWindowTitle(this->module_name);
    this->grader_editor_widget=new grader_editor(this,this->project_path,this->module_name,this->filesList,this->marks_denominations);
    this->grader_editor_widget->show();
    this->setFixedWidth(this->grader_editor_widget->size().width());
    this->setFixedHeight(this->grader_editor_widget->size().height());
}

