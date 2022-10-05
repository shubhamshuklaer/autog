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
#include <QDoubleValidator>
#include <QWidget>

#include "grader_marks_widget.h"
#include "ui_grader_marks_widget.h"


grader_marks_widget::grader_marks_widget( QWidget *parent,
                                       QStringList marks_denominations_list ) :
    QWidget(parent),
    ui(new Ui::grader_marks_widget)
{
    int i;

    ui->setupUi(this);

    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocusProxy(this->ui->marks_text);
    this->marks_check_boxes_list=QList<QCheckBox *>();
    this->ui->marks_text->setValidator(new QDoubleValidator(this));
    i=0;

    foreach( QString mark , marks_denominations_list ){
        if( mark.toFloat() != 0 ){
            QCheckBox *new_box=new QCheckBox();
            new_box->setText(mark);
            this->ui->horizontal_layout->addWidget(new_box);
            this->marks_check_boxes_list<<new_box;

            // I am using clicked signal from QABstractButton as statechanged is
            //aslo emited when state is changed programitically

            connect(new_box,SIGNAL( clicked(bool)), this,
                                             SLOT( check_box_state_changed() ) );
            if(i==0)
                QWidget::setTabOrder( this->ui->marks_text,
                                            this->marks_check_boxes_list[i] );
            else
                QWidget::setTabOrder( this->marks_check_boxes_list[i-1],
                                                this->marks_check_boxes_list[i] );
        }
    }

    connect( this->ui->marks_text, SIGNAL( textEdited( QString ) ), this, SLOT( marks_edited() ) );
}

grader_marks_widget::~grader_marks_widget()
{
    delete ui;
}

QString grader_marks_widget::get_marks(){
    QString marks;
    float marks_value=0;
    foreach( QCheckBox *check_box , this->marks_check_boxes_list ){
        if( check_box->isChecked() ){
            marks+=check_box->text()+"+";
            marks_value+=check_box->text().toFloat();
        }else{
            marks+="0+";
        }
    }
    if( QString::number( marks_value ) == this->ui->marks_text->text().trimmed() ){
        marks.truncate( marks.size()-1 );   //removing the last +
    }else{
        marks=this->ui->marks_text->text();
    }

    return marks;
}

void grader_marks_widget::put_marks(QString marks){
    QStringList marks_denominations=marks.split( "+", Qt::SkipEmptyParts );
    float marks_value=0;
    int i=0;

    bool wrong_format=false;
    if( marks_denominations.size() != this->marks_check_boxes_list.size() )
        wrong_format=true;

    while( i < marks_denominations.size() ){
        marks_value+=marks_denominations[i].toFloat();
        if( !wrong_format ){
            if( marks_denominations[i].trimmed() == "0" ){
                this->marks_check_boxes_list[i]->setChecked(false);
            }else if( marks_denominations[i].trimmed() == this->marks_check_boxes_list[i]->text() ){
                this->marks_check_boxes_list[i]->setChecked(true);
            }else{
                wrong_format=true;
            }
        }
        i++;
    }

    //Resetting marks checkboxes if the format is wrong
    if( wrong_format ){
        foreach( QCheckBox *check_box , this->marks_check_boxes_list )
            check_box->setChecked(false);
    }

    this->ui->marks_text->setText(QString::number(marks_value));
}

void grader_marks_widget::check_box_state_changed(){
    float marks=0;
    foreach( QCheckBox *check_box , this->marks_check_boxes_list ){
        if( check_box->isChecked() )
            marks+=check_box->text().toFloat();
    }
    this->ui->marks_text->setText(QString::number(marks));
    emit marks_changed();
}

void grader_marks_widget::marks_edited(){
    foreach( QCheckBox *check_box, this->marks_check_boxes_list )
        check_box->setChecked( false );
    emit marks_changed();
}
