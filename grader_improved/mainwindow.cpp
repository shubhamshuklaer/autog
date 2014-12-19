/*
 *  Copyright (C) 2014 Shubham Shukla <shubham.shukla@iitg.ernet.in>
 *  This file is part of Auto Grader.
 *
 *  Auto Grader is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Auto Grader is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Auto Grader.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "grader_editor.h"
#include "grader_project_load.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->project_load_widget=new grader_project_load(this);
    this->ui->centralWidget->setFixedSize(this->project_load_widget->size());
    connect( this->project_load_widget, SIGNAL(done()),this,SLOT(setup_done()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_done(){
    this->project_path=this->project_load_widget->property("project_path").toString();
    this->module_name=this->project_load_widget->property("module_name").toString();
    this->files_list=this->project_load_widget->property("files_list").toStringList();
    this->marks_denominations_list=this->project_load_widget->property(
                "marks_denominations").toStringList();
    delete this->project_load_widget;
    this->setWindowTitle(this->module_name);
    this->editor_widget=new grader_editor(this,this->project_path,this->module_name,
                                          this->files_list,this->marks_denominations_list);
    this->editor_widget->show();
    this->setFixedWidth(this->editor_widget->size().width());
    this->setFixedHeight(this->editor_widget->size().height());
}

