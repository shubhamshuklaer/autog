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

#include <QCompleter>
#include <QCoreApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QMetaMethod>
#include <QMutex>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>
#include <QUrl>

#include "constants.h"
#include "grader_combo_validator.h"
#include "grader_editor.h"
#include "grader_marks_widget.h"
#include "ui_grader_editor.h"


grader_editor::grader_editor(QWidget *parent,QString project_path,QString module_name,QStringList files_list,QStringList marks_denominations) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);
    this->project_path=project_path;
    this->module_name=module_name;
    this->files_list=files_list;
    this->marks_denominations=marks_denominations;

    this->out_dir_name=project_path+"/"+module_name+"/"+const_out_dir_name;
    this->main_tex_dir_name=project_path+"/"+module_name;
    this->current_index=0;
    this->sub_tex_name=project_path+"/"+const_sub_tex_name;
    this->ui->file_name_combo->addItems(this->files_list);
    this->ui->file_name_combo->setCurrentIndex(0);
    this->marks_widget=NULL;

    this->file_sys_thread= new QThread(this);
    this->file_sys_interface=new grader_file_sys(NULL,this->main_tex_dir_name,this->out_dir_name);
    //QObject::moveToThread: Cannot move objects with a parent
    this->file_sys_interface->moveToThread(this->file_sys_thread);
    connect(this->file_sys_interface,SIGNAL(send_error(QString)),this,SLOT(set_tex_error(QString)));
    this->file_sys_thread->start();

    setup_marks_widget(0);

    this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->files_list[0],"t"));
    if(this->current_index+1>=this->files_list.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(false);

    QCompleter *completer;
    completer=new QCompleter(this->ui->file_name_combo);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->ui->file_name_combo->model());
    this->ui->file_name_combo->setCompleter(completer);
    this->ui->file_name_combo->setValidator(new grader_combo_validator(this->ui->file_name_combo,this->ui->file_name_combo->model()));


    completer=new QCompleter(this->ui->comment_pos_combo);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->ui->comment_pos_combo->model());
    this->ui->comment_pos_combo->setCompleter(completer);
    this->ui->comment_pos_combo->setValidator(new grader_combo_validator(this->ui->comment_pos_combo,this->ui->comment_pos_combo->model()));

}

grader_editor::~grader_editor()
{
    this->file_sys_thread->quit();
    this->file_sys_thread->wait();
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    generate_pdf(false,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->tex_errors!=NULL){
        QMessageBox::warning(
                    this,
                    tr("Grader"),
                    tr("Please fix all errors before proceding"));
    }else{
        if(this->current_index+2==this->files_list.length())
            this->ui->next_btn->setEnabled(false);
        this->ui->prev_btn->setEnabled(true);
        this->current_index++;
        this->ui->file_name_combo->setCurrentIndex(this->current_index);
        setup_marks_widget(this->current_index);
        this->ui->comment_pos_combo->setCurrentIndex(0);
        this->previous_comment_pos_index=0;
        this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->files_list[this->current_index],"t"));
    }
}

void grader_editor::on_prev_btn_clicked()
{
    generate_pdf(false,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->tex_errors!=NULL){
        QMessageBox::warning(
                    this,
                    tr("Grader"),
                    tr("Please fix all errors before proceding"));
    }else{
        if(this->current_index<=1)
            this->ui->prev_btn->setEnabled(false);
        this->ui->next_btn->setEnabled(true);
        this->current_index--;
        this->ui->file_name_combo->setCurrentIndex(this->current_index);
        setup_marks_widget(this->current_index);
        this->ui->comment_pos_combo->setCurrentIndex(0);
        this->previous_comment_pos_index=0;
        this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->files_list[this->current_index],"t"));
    }
}

void grader_editor::on_preview_btn_clicked()
{
    generate_pdf(false,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(!QDesktopServices::openUrl(QUrl("file:///"+this->main_tex_dir_name+"/"+const_main_pdf_name+".pdf", QUrl::TolerantMode))){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+this->main_tex_dir_name+"/"+const_main_pdf_name+".pdf");
    }
}


void grader_editor::on_file_name_combo_activated(int index)
{
    if(index!=this->current_index){
        generate_pdf(false,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
        if(this->tex_errors!=NULL){
            this->ui->file_name_combo->setCurrentIndex(this->current_index);
            QMessageBox::warning(
                        this,
                        tr("Grader"),
                        tr("Please fix all errors before proceding"));
        }else{
            this->current_index=index;
            if(this->current_index==0)
                this->ui->prev_btn->setEnabled(false);
            else
                this->ui->prev_btn->setEnabled(true);
            if(this->current_index+1>=this->files_list.length())
                this->ui->next_btn->setEnabled(false);
            else
                this->ui->next_btn->setEnabled(true);
            setup_marks_widget(this->current_index);
            this->ui->comment_pos_combo->setCurrentIndex(0);
            this->previous_comment_pos_index=0;
            this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->files_list[this->current_index],"t"));
        }
    }
}


void grader_editor::on_fix_file_btn_clicked()
{
    QDir out_dir(this->out_dir_name);
    if(out_dir.exists(this->files_list[this->current_index]+".tex")){
        if(!out_dir.remove(this->files_list[this->current_index]+".tex")){
            QMessageBox::warning(
                        this,
                        tr("Error"),
                        tr("couldn't file ")+this->out_dir_name+"/"+this->files_list[this->current_index]+".tex"+tr(" exists and couldn't be removed for overwriting"));
            return;
        }
    }

    if(!QFile::copy(this->sub_tex_name,this->out_dir_name+"/"+this->files_list[this->current_index]+".tex")){
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("couldn't copy file from ")+this->sub_tex_name+tr(" to ")+this->out_dir_name+"/"+this->files_list[this->current_index]+".tex");
        return;
    }
    //putting put page

    QFile sub_tex_file(this->out_dir_name+"/"+this->files_list[this->current_index]+".tex");

    if(!sub_tex_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+this->out_dir_name+"/"+this->files_list[this->current_index]+".tex"+tr(" for read"));
        return;
    }

    QTextStream sub_tex_input_stream(&sub_tex_file);
    QString sub_tex_content=sub_tex_input_stream.readAll();
    sub_tex_file.close();

    if(!sub_tex_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+this->out_dir_name+"/"+this->files_list[this->current_index]+".tex"+tr(" for read"));
        return;
    }


    QTextStream sub_tex_output_stream(&sub_tex_file);
    QRegularExpression put_page_pattern("\\\\putpage{.*");
    qDebug()<<"Sub tex content"<<sub_tex_content;
    sub_tex_content.replace(put_page_pattern,"\\putpage{"+this->files_list[this->current_index]+"}");
    qDebug()<<sub_tex_content;
    sub_tex_output_stream<<sub_tex_content;

    sub_tex_file.flush();
    sub_tex_file.close();

    generate_pdf(false,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
}



void grader_editor::on_see_errors_btn_clicked()
{
    QString temp_tex_errors;
    temp_tex_errors=this->tex_errors;
    QMessageBox::warning(
                this,
                tr("Grader"),
                temp_tex_errors );
}



void grader_editor::setup_marks_widget(int index){
    if(this->marks_widget!=NULL)
        delete this->marks_widget;
    this->marks_widget=new grader_marks_widget(this->ui->marks_widget,this->marks_denominations[index].split(marks_denominations_delemiter));
    this->marks_widget->setProperty("marks",this->file_sys_interface->get_marks(this->files_list[index]));
    this->marks_widget->setFixedSize(this->ui->marks_widget->size());
    connect(this->marks_widget,SIGNAL(marks_changed()),this,SLOT(on_marks_text_textChanged()));
    this->ui->marks_label->setBuddy(this->marks_widget);
    this->marks_widget->show();
}



void grader_editor::on_open_tex_btn_clicked()
{
    if(!QDesktopServices::openUrl(QUrl("file:///"+this->out_dir_name+"/"+this->files_list[this->current_index]+".tex", QUrl::TolerantMode))){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+this->out_dir_name+"/"+this->files_list[this->current_index]+".tex");
    }
}

void grader_editor::on_marks_text_textChanged(){
    generate_pdf(true,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
}


void grader_editor::on_comment_text_textChanged()
{
    generate_pdf(true,this->files_list[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
}

void grader_editor::on_comment_pos_combo_activated(int index)
{
    put_comment(false,this->files_list[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->previous_comment_pos_index));
    this->previous_comment_pos_index=index;
    this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->files_list[this->current_index],this->ui->comment_pos_combo->itemText(index)));
}




void grader_editor::put_marks(bool async,QString file_name , QString marks){
    if(async){
        QByteArray normalizedSignature = QMetaObject::normalizedSignature("put_comment(QString, QString)");
        int method_index=this->file_sys_interface->metaObject()->indexOfMethod(normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->method(method_index);
        method.invoke(this->file_sys_interface,Qt::QueuedConnection,Q_ARG(QString,file_name),Q_ARG(QString,marks));
    }else{
        this->file_sys_interface->put_marks(file_name,marks);
    }
}



void grader_editor::put_comment(bool async,QString file_name , QString comment,QString comment_pos){
    if(async){
        QByteArray normalizedSignature = QMetaObject::normalizedSignature("put_comment(QString, QString)");
        int method_index=this->file_sys_interface->metaObject()->indexOfMethod(normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->method(method_index);
        method.invoke(this->file_sys_interface,Qt::QueuedConnection,Q_ARG(QString, file_name),Q_ARG(QString, comment),Q_ARG(QString, comment_pos));
    }else{
        this->file_sys_interface->put_comment(file_name,comment,comment_pos);
    }
}


void grader_editor::generate_pdf(bool async,QString file_name,QString marks,QString comment_text,QString comment_pos){
    if(async){
        QByteArray normalizedSignature = QMetaObject::normalizedSignature("generate_pdf(QString, QString,QString,QString)");
        int method_index=this->file_sys_interface->metaObject()->indexOfMethod(normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->method(method_index);
        QCoreApplication::removePostedEvents(this->file_sys_interface);
        method.invoke(this->file_sys_interface,Qt::QueuedConnection,Q_ARG(QString,file_name),Q_ARG(QString,marks),Q_ARG(QString,comment_text),Q_ARG(QString,comment_pos));
    }else{
        this->file_sys_interface->generate_pdf(file_name,marks,comment_text,comment_pos);
    }
}

void grader_editor::set_tex_error(QString error){
    this->tex_errors_lock.lock();
    this->tex_errors=error;
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
}
