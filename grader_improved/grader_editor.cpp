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
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QMetaMethod>
#include <QMutex>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

#include "constants.h"
#include "grader_combo_validator.h"
#include "grader_editor.h"
#include "grader_marks_widget.h"
#include "ui_grader_editor.h"
#include "lightlatexhighlighter.h"


grader_editor::grader_editor( QWidget *parent, QString project_path,
                              QString module_name, QStringList files_list,
                                        QStringList marks_denominations_list,
                              QList<QStringList> merge_list,int start_grading_from ) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);
    this->project_path=project_path;
    this->module_name=module_name;
    this->files_list=files_list;
    this->marks_denominations_list=marks_denominations_list;
    this->merge_list=merge_list;


    this->module_dir_path=project_path+"/"+module_name;
    this->current_index=start_grading_from;
    this->ui->file_name_combo->addItems(this->files_list);
    this->ui->file_name_combo->setCurrentIndex(start_grading_from);
    this->marks_widget=NULL;

    this->file_sys_thread= new QThread(this);
    //I have set the parent as null cause we can't move a object
    //with parent to a different thread
    this->file_sys_interface=new grader_file_sys(NULL, this->module_dir_path );
    //QObject::moveToThread: Cannot move objects with a parent
    this->file_sys_interface->moveToThread(this->file_sys_thread);
    connect( this->file_sys_interface, SIGNAL( send_tex_compile_error( QString ) ),
                                this, SLOT( set_tex_compile_error( QString ) ) );
    connect( this->file_sys_interface, SIGNAL( send_error( QString ) ),
                                  this, SLOT( display_errors_slot( QString ) ) );
    this->file_sys_thread->start();

    this->merge_combo_box=NULL;
    this->current_merge_index=0;
    setup_merge_widget(this->current_index);
    setup_marks_widget(this->current_index);


    this->ui->comment_text->setText(this->file_sys_interface->get_comment(
                                    this->files_list[this->current_index], this->ui->comment_pos_combo->itemText(0),get_merge_index() ) );

    if(this->current_index+1>=this->files_list.length())
        this->ui->next_btn->setEnabled(false);
    if( this->current_index == 0 )
        this->ui->prev_btn->setEnabled(false);

    QCompleter *completer;
    completer=new QCompleter(this->ui->file_name_combo);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->ui->file_name_combo->model());
    completer->setFilterMode( Qt::MatchContains );
    this->ui->file_name_combo->setCompleter(completer);
    this->ui->file_name_combo->setValidator(
                new grader_combo_validator( this->ui->file_name_combo,
                                          this->ui->file_name_combo->model() ) );


    completer=new QCompleter(this->ui->comment_pos_combo);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->ui->comment_pos_combo->model());
    completer->setFilterMode( Qt::MatchContains );
    this->ui->comment_pos_combo->setCompleter(completer);
    this->ui->comment_pos_combo->setValidator(
                new grader_combo_validator( this->ui->comment_pos_combo,
                                        this->ui->comment_pos_combo->model() ) );

    LightLatexHighlighter *highlighter=new LightLatexHighlighter(
                                            this->ui->comment_text->document() );

}

grader_editor::~grader_editor()
{
    this->file_sys_thread->quit();
    this->file_sys_thread->wait();
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    generate_pdf( false, this->files_list[this->current_index],
            this->marks_widget->property( "marks" ).toString(),
                        this->ui->comment_text->toPlainText(),
                            this->ui->comment_pos_combo->itemText(
                                this->ui->comment_pos_combo->currentIndex() ) );

    if(this->tex_compile_errors!=NULL){
        display_error( tr( "Please fix all errors before proceding" ) );
    }else{
        if(this->current_index+2==this->files_list.length())
            this->ui->next_btn->setEnabled(false);
        this->ui->prev_btn->setEnabled(true);
        this->current_index++;
        this->ui->file_name_combo->setCurrentIndex(this->current_index);
        setup_merge_widget(this->current_index);
        setup_marks_widget(this->current_index);
        this->ui->comment_pos_combo->setCurrentIndex(0);
        this->previous_comment_pos_index=0;
        this->ui->comment_text->setText(this->file_sys_interface->
                                  get_comment(
                                      this->files_list[this->current_index],
                                                                        this->ui->comment_pos_combo->itemText(0),get_merge_index()) );

        generate_pdf( false, this->files_list[this->current_index],
                this->marks_widget->property( "marks" ).toString(),
                            this->ui->comment_text->toPlainText(),
                                this->ui->comment_pos_combo->itemText(
                                    this->ui->comment_pos_combo->currentIndex() ) );
    }
}

void grader_editor::on_prev_btn_clicked()
{
    generate_pdf( false, this->files_list[this->current_index],
            this->marks_widget->property( "marks" ).toString(),
                        this->ui->comment_text->toPlainText(),
                            this->ui->comment_pos_combo->itemText(
                                this->ui->comment_pos_combo->currentIndex() ) );

    if( this->tex_compile_errors != NULL ){
        display_error( tr( "Please fix all errors before proceding" ) );
    }else{
        if(this->current_index<=1)
            this->ui->prev_btn->setEnabled(false);
        this->ui->next_btn->setEnabled(true);
        this->current_index--;
        this->ui->file_name_combo->setCurrentIndex(this->current_index);
        setup_merge_widget(this->current_index);
        setup_marks_widget(this->current_index);
        this->ui->comment_pos_combo->setCurrentIndex(0);
        this->previous_comment_pos_index=0;
        this->ui->comment_text->setText(this->file_sys_interface->
                                  get_comment(
                                      this->files_list[this->current_index],
                                                                        this->ui->comment_pos_combo->itemText(0),get_merge_index()) );

        generate_pdf( false, this->files_list[this->current_index],
                this->marks_widget->property( "marks" ).toString(),
                            this->ui->comment_text->toPlainText(),
                                this->ui->comment_pos_combo->itemText(
                                    this->ui->comment_pos_combo->currentIndex() ) );
    }
}

void grader_editor::on_preview_btn_clicked()
{
    generate_pdf( false, this->files_list[this->current_index],
            this->marks_widget->property( "marks" ).toString(),
                        this->ui->comment_text->toPlainText(),
                            this->ui->comment_pos_combo->itemText(
                                this->ui->comment_pos_combo->currentIndex() ) );


    this->file_sys_interface->open_pdf();
}


void grader_editor::on_file_name_combo_activated(int index)
{
    if(index!=this->current_index){

        generate_pdf( false, this->files_list[this->current_index],
                this->marks_widget->property( "marks" ).toString(),
                            this->ui->comment_text->toPlainText(),
                                this->ui->comment_pos_combo->itemText(
                                    this->ui->comment_pos_combo->currentIndex() ) );

        //Will only allow him to go ahed if this page is free of errors
        if(this->tex_compile_errors!=NULL){
            this->ui->file_name_combo->setCurrentIndex(this->current_index);
            display_error( tr( "Please fix all errors before proceding" ) );
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

            setup_merge_widget(this->current_index);
            setup_marks_widget(this->current_index);


            this->ui->comment_pos_combo->setCurrentIndex(0);
            this->previous_comment_pos_index=0;

            this->ui->comment_text->setText( this->file_sys_interface->get_comment(
                                       this->files_list[this->current_index], this->ui->comment_pos_combo->itemText(0),get_merge_index() ) );

            generate_pdf( false, this->files_list[this->current_index],
                    this->marks_widget->property( "marks" ).toString(),
                                this->ui->comment_text->toPlainText(),
                                    this->ui->comment_pos_combo->itemText(
                                        this->ui->comment_pos_combo->currentIndex() ) );
        }
    }
}


void grader_editor::on_merge_combo_activated(int index){
    if(index!=this->current_merge_index){
        generate_pdf( false, this->files_list[this->current_index],
                this->marks_widget->property( "marks" ).toString(),
                            this->ui->comment_text->toPlainText(),
                                this->ui->comment_pos_combo->itemText(
                                    this->ui->comment_pos_combo->currentIndex() ) );
        this->current_merge_index=index;
        setup_marks_widget(this->current_index);
        this->ui->comment_pos_combo->setCurrentIndex(0);
        this->previous_comment_pos_index=0;

        this->ui->comment_text->setText( this->file_sys_interface->get_comment(
                                   this->files_list[this->current_index], this->ui->comment_pos_combo->itemText(0),get_merge_index() ) );

    }
}



void grader_editor::on_see_errors_btn_clicked()
{
    display_error( this->tex_compile_errors );
}



void grader_editor::setup_marks_widget(int index){
    //The tab order is set up in setup_marks_widget only cause every time
    //after setup_merge_widget is called this function is called too.
    if(this->marks_widget!=NULL)
        delete this->marks_widget;

    this->marks_widget=new grader_marks_widget( this->ui->marks_widget,
                                        this->marks_denominations_list[index].split(
                                                    marks_denominations_delemiter ) );
    this->marks_widget->setProperty("marks",this->file_sys_interface->get_marks(
                                                          this->files_list[index],get_merge_index() ) );
    this->marks_widget->setFixedSize(this->ui->marks_widget->size());
    connect( this->marks_widget, SIGNAL( marks_changed() ), this, SLOT(
                                                      on_marks_text_textChanged() ) );
    this->ui->marks_label->setBuddy(this->marks_widget);

    //setting tab order of editor widget
    if(this->merge_combo_box!=NULL){
        QWidget::setTabOrder( this->ui->file_name_combo, this->merge_combo_box );
        QWidget::setTabOrder( this->merge_combo_box, this->marks_widget );
    }else{
        QWidget::setTabOrder( this->ui->file_name_combo, this->marks_widget );
    }

    QWidget::setTabOrder( this->marks_widget, this->ui->comment_pos_combo );
    QWidget::setTabOrder( this->ui->comment_pos_combo, this->ui->comment_text );
    QWidget::setTabOrder( this->ui->comment_text, this->ui->next_btn );
    QWidget::setTabOrder( this->ui->next_btn, this->ui->prev_btn );
    QWidget::setTabOrder( this->ui->prev_btn, this->ui->see_errors_btn );
    QWidget::setTabOrder( this->ui->see_errors_btn, this->ui->open_tex_btn );
    QWidget::setTabOrder( this->ui->open_tex_btn, this->ui->preview_btn );


    this->marks_widget->show();
}



void grader_editor::on_open_tex_btn_clicked()
{
    this->file_sys_interface->open_tex_file(this->files_list[this->current_index]);
}

void grader_editor::on_marks_text_textChanged(){
    generate_pdf( true, this->files_list[this->current_index],
            this->marks_widget->property( "marks" ).toString(),
                        this->ui->comment_text->toPlainText(),
                            this->ui->comment_pos_combo->itemText(
                                this->ui->comment_pos_combo->currentIndex() ) );
}


void grader_editor::on_comment_text_textChanged()
{
    generate_pdf( true, this->files_list[this->current_index],
            this->marks_widget->property( "marks" ).toString(),
                        this->ui->comment_text->toPlainText(),
                            this->ui->comment_pos_combo->itemText(
                                this->ui->comment_pos_combo->currentIndex() ) );
}


void grader_editor::on_comment_pos_combo_activated(int index)
{
    put_comment( false, this->files_list[this->current_index],
                    this->ui->comment_text->toPlainText(),
                                this->ui->comment_pos_combo->itemText(
                                            this->previous_comment_pos_index ) );

    this->previous_comment_pos_index=index;
    this->ui->comment_text->setText(this->file_sys_interface->get_comment(
                                        this->files_list[this->current_index],
                                this->ui->comment_pos_combo->itemText( index ),get_merge_index() ) );
}



void grader_editor::put_comment( bool async, QString file_name ,
                                        QString comment , QString comment_pos ){
    int index=get_merge_index();
    if(async){
        //Asynchronous function call
        //the call is posted as a event the the file sys interface thread
        QByteArray normalizedSignature = QMetaObject::normalizedSignature(
                                                "put_comment( QString, QString )" );
        int method_index=this->file_sys_interface->metaObject()->
                                                  indexOfMethod(normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->
                                                                method(method_index);
        method.invoke( this->file_sys_interface, Qt::QueuedConnection,
                            Q_ARG( QString, file_name ), Q_ARG( QString, comment ),
                                                     Q_ARG( QString, comment_pos ), Q_ARG( int, index ) );
    }else{
        this->file_sys_interface->put_comment(file_name,comment,comment_pos,index);
    }
}


void grader_editor::generate_pdf(bool async, QString file_name, QString marks,
                                        QString comment_text, QString comment_pos ){
    int index=get_merge_index();
    if(async){
        //Asynchronous function call
        //the call is posted as a event the the file sys interface thread
        QByteArray normalizedSignature = QMetaObject::normalizedSignature(
                            "generate_pdf( QString, QString, QString, QString, int )" );
        int method_index=this->file_sys_interface->metaObject()->indexOfMethod(
                                                                normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->method(
                                                                      method_index);
        QCoreApplication::removePostedEvents(this->file_sys_interface);
        method.invoke(this->file_sys_interface, Qt::QueuedConnection,
                            Q_ARG( QString, file_name ), Q_ARG( QString, marks ),
                                        Q_ARG( QString, comment_text ),
                                                  Q_ARG( QString, comment_pos ), Q_ARG( int, index ) );
    }else{
        this->file_sys_interface->generate_pdf( file_name, marks, comment_text,
                                                                    comment_pos, index);
    }
}


void grader_editor::set_tex_compile_error(QString error){
    this->tex_compile_errors_lock.lock();

    this->tex_compile_errors=error;

    //Setting button stylesheet depending on
    //if error is not empty
    if(this->tex_compile_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }

    this->tex_compile_errors_lock.unlock();
}


void grader_editor::display_errors_slot(QString error){
    display_error(error);
}


void grader_editor::display_error( QString error ){
    QMessageBox::critical( this, tr( "Error" ), error );
}

void grader_editor::setup_merge_widget(int index){
    if(this->merge_combo_box!=NULL){
        delete this->merge_combo_box;
        this->merge_combo_box=NULL;
        this->current_merge_index=0;
    }

    if(this->merge_list[index].isEmpty())
        return;

    this->merge_combo_box=new QComboBox(this);
    this->merge_combo_box->setEditable(true);
    this->merge_combo_box->setAcceptDrops(false);
    this->ui->id_merge_layout->addWidget(this->merge_combo_box);
    this->merge_combo_box->addItems(this->merge_list[index]);
    this->current_merge_index=0;

    QCompleter * completer=new QCompleter(this->merge_combo_box);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->merge_combo_box->model());
    completer->setFilterMode( Qt::MatchContains );
    this->merge_combo_box->setCompleter(completer);
    this->merge_combo_box->setValidator(
                new grader_combo_validator( this->merge_combo_box,
                                        this->merge_combo_box->model() ) );

    connect( this->merge_combo_box, SIGNAL( activated(int) ), this, SLOT(
                                                      on_merge_combo_activated(int) ) );
    this->merge_combo_box->show();


}

int grader_editor::get_merge_index(){
    if(this->merge_combo_box==NULL)
        return 0;
    else
        return this->current_merge_index;
}
