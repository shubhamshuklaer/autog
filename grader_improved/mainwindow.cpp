#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "grader_setup.h"
#include "grader_editor.h"
#include "grader_settings.h"
#include <QDebug>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->grader_settings_widget=new grader_settings();
    this->grader_setup_widget=new grader_setup(this);
    this->ui->centralWidget->setFixedSize(this->grader_setup_widget->size());
    connect( this->ui->config, SIGNAL(triggered()), this, SLOT(open_config_window()));
    connect( this->grader_setup_widget, SIGNAL(done()),this,SLOT(setup_done()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::open_config_window(){
    grader_settings_widget->show();
}

void MainWindow::setup_done(){
    this->out_dir_name=this->grader_setup_widget->out_dir_name;
    this->filesList=this->grader_setup_widget->filesList;
    delete this->grader_setup_widget;
    this->grader_editor_widget=new grader_editor(this,this->filesList,this->out_dir_name);
    this->grader_editor_widget->show();
    this->ui->centralWidget->setFixedSize(this->grader_editor_widget->size());
}
