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

#include <QCheckBox>
#include <QDebug>
#include <QWidget>

#include "grader_marks_widget.h"
#include "ui_grader_marks_widget.h"


grader_marks_widget::grader_marks_widget(QWidget *parent,QStringList marks_denominations) :
    QWidget(parent),
    ui(new Ui::grader_marks_widget)
{
    ui->setupUi(this);
    this->ui->marks_text->setValidator(new QDoubleValidator(this));
    this->check_box_list=QList<QCheckBox *>();
    int i=0;
    foreach(QString mark,marks_denominations){
        qDebug() <<"marks "<<mark;
        if(mark.toFloat()!=0){
            qDebug() <<mark.toFloat();
            QCheckBox *temp_box=new QCheckBox();
            temp_box->setText(mark);
            this->ui->horizontal_layout->addWidget(temp_box);
            this->check_box_list<<temp_box;
            connect(temp_box,SIGNAL(stateChanged(int)),this,SLOT(marks_check_box_changed()));
            if(i==0)
                QWidget::setTabOrder(this->ui->marks_text,this->check_box_list[i]);
            else
                QWidget::setTabOrder(this->check_box_list[i-1],this->check_box_list[i]);
        }
    }
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocusProxy(this->ui->marks_text);
    connect(this->ui->marks_text,SIGNAL(textChanged(QString)),this,SIGNAL(marks_changed()));
}

grader_marks_widget::~grader_marks_widget()
{
    delete ui;
}

QString grader_marks_widget::get_marks(){
    return this->ui->marks_text->text();
}

void grader_marks_widget::put_marks(QString marks){
    this->ui->marks_text->setText(marks);
}

void grader_marks_widget::marks_check_box_changed(){
    float marks=0;
    foreach(QCheckBox *check_box,this->check_box_list){
        if(check_box->isChecked())
            marks+=check_box->text().toFloat();
    }
    qDebug() <<marks;
    qDebug() <<QString::number(marks);
    this->ui->marks_text->setText(QString::number(marks));
}
