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
#include <QHBoxLayout>

#include "constants.h"
#include "grader_combo_validator.h"
#include "grader_editor.h"
#include "grader_marks_widget.h"
#include "ui_grader_editor.h"
#include "lightlatexhighlighter.h"


grader_editor::grader_editor( QWidget *parent, QString project_path,
                              QString module_name, QStringList files_list,
                                                    int start_grading_from ) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);

    this->project_path=project_path;
    this->module_name=module_name;
    this->files_list=files_list;
    this->current_index=start_grading_from;
    this->module_dir_path=project_path+"/"+module_name;


    this->ui->file_name_combo->addItems(this->files_list);
    this->ui->file_name_combo->setCurrentIndex(start_grading_from);
    int width = this->ui->file_name_combo->minimumSizeHint().width();//width of largest item
    this->ui->file_name_combo->setMinimumWidth(width);
    this->ui->marks_widget->setLayout(new QHBoxLayout(this->ui->marks_widget));
    this->ui->marks_widget->layout()->setContentsMargins(0,0,0,0);
    set_combobox_completer(this->ui->file_name_combo);
    set_combobox_completer(this->ui->comment_pos_combo);


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
    this->marks_widget=NULL;
    this->current_merge_index=0;
    this->current_comment_pos_index=0;
    this->tex_compile_errors=QString();

    this->generate_pdf_timer = new QTimer(this);
    connect(this->generate_pdf_timer, &QTimer::timeout, this, &grader_editor::generatePdfTimerTimeout);

    load_page(true,this->current_index);

    new LightLatexHighlighter(this->ui->comment_text->document() );

}

grader_editor::~grader_editor()
{
    this->file_sys_thread->quit();
    this->file_sys_thread->wait();
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    load_page(false,this->current_index+1);
}

void grader_editor::on_prev_btn_clicked()
{
    load_page(false,this->current_index-1);
}

void grader_editor::on_preview_btn_clicked()
{
    generate_pdf( false );
    this->file_sys_interface->open_pdf();
}


void grader_editor::on_file_name_combo_activated(int index)
{
    load_page(false,index);
}


void grader_editor::on_merge_combo_activated(int index){
    if(index!=this->current_merge_index){
        put_comment( false );
        put_marks(false);

        this->current_merge_index=index;
        setup_marks_widget();

        this->ui->comment_text->setText( get_comment() );

    }
}



void grader_editor::on_see_errors_btn_clicked()
{
    display_error( this->tex_compile_errors );
}



void grader_editor::setup_marks_widget(){
    //The tab order is set up in setup_marks_widget only cause every time
    //after setup_merge_widget is called this function is called too.
    if(this->marks_widget!=NULL)
        delete this->marks_widget;

    this->marks_widget=new grader_marks_widget( this->ui->marks_widget,
                                        this->current_marking_scheme.split(
                                                    marks_denominations_delemiter ) );
    this->marks_widget->setProperty("marks",get_marks() );
    this->ui->marks_widget->layout()->addWidget(this->marks_widget);
    //    this->marks_widget->setFixedSize(this->ui->marks_widget->size());
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

void grader_editor::generatePdfTimerTimeout(){
    generate_pdf( true );
}

void grader_editor::on_marks_text_textChanged(){
    this->generate_pdf_timer->start(const_generate_pdf_timer_interval_msecs);
}


void grader_editor::on_comment_text_textChanged()
{
    this->generate_pdf_timer->start(const_generate_pdf_timer_interval_msecs);
}


void grader_editor::on_comment_pos_combo_activated(int index)
{
    put_comment( false );
    this->current_comment_pos_index=index;
    this->ui->comment_text->setText( get_comment() );
}




void grader_editor::put_marks( bool async ){

    QString file_name,marks;

    int index=get_merge_index();

    file_name=this->files_list[this->current_index];
    marks=this->marks_widget->property( "marks" ).toString();

    if(async){
        //Asynchronous function call
        //the call is posted as a event the the file sys interface thread
        QByteArray normalizedSignature = QMetaObject::normalizedSignature(
                                                "put_marks( QString, QString, int )" );
        int method_index=this->file_sys_interface->metaObject()->
                                                  indexOfMethod(normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->
                                                                method(method_index);
        method.invoke( this->file_sys_interface, Qt::QueuedConnection,
                            Q_ARG( QString, file_name ), Q_ARG( QString, marks )
                                                     , Q_ARG( int, index ) );
    }else{
        this->file_sys_interface->put_marks(file_name,marks,index);
    }
}




void grader_editor::put_comment( bool async ){

    QString file_name,comment_text,comment_pos;
    int index=get_merge_index();

    file_name=this->files_list[this->current_index];
    comment_text=this->ui->comment_text->toPlainText();
    comment_pos=this->ui->comment_pos_combo->itemText(this->current_comment_pos_index);

    if(async){
        //Asynchronous function call
        //the call is posted as a event the the file sys interface thread
        QByteArray normalizedSignature = QMetaObject::normalizedSignature(
                                                "put_comment( QString, QString, QString, int )" );
        int method_index=this->file_sys_interface->metaObject()->
                                                  indexOfMethod(normalizedSignature);
        QMetaMethod method=this->file_sys_interface->metaObject()->
                                                                method(method_index);
        method.invoke( this->file_sys_interface, Qt::QueuedConnection,
                            Q_ARG( QString, file_name ), Q_ARG( QString, comment_text ),
                                                     Q_ARG( QString, comment_pos ), Q_ARG( int, index ) );
    }else{
        this->file_sys_interface->put_comment(file_name,comment_text,comment_pos,index);
    }
}


void grader_editor::generate_pdf(bool async ){
    this->generate_pdf_timer->stop();
    QString file_name,marks,comment_text,comment_pos;
    int index=get_merge_index();

    file_name=this->files_list[this->current_index];
    marks=this->marks_widget->property( "marks" ).toString();
    comment_text=this->ui->comment_text->toPlainText();
    comment_pos=this->ui->comment_pos_combo->itemText(this->current_comment_pos_index);

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

void grader_editor::setup_merge_widget(){
    if(this->merge_combo_box!=NULL){
        delete this->merge_combo_box;
        delete this->merge_label;
        this->merge_combo_box=NULL;
        this->current_merge_index=0;
    }

    if(this->current_merge_list.isEmpty())
        return;

    this->merge_combo_box=new QComboBox(this);
    this->merge_combo_box->setEditable(true);
    this->merge_combo_box->setAcceptDrops(false);
    this->merge_combo_box->addItems(this->current_merge_list);
    this->merge_combo_box->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
    int width = this->merge_combo_box->minimumSizeHint().width();//width of largest item
    this->merge_combo_box->setMinimumWidth(width);
    this->current_merge_index=0;

    set_combobox_completer(this->merge_combo_box);

    connect( this->merge_combo_box, SIGNAL( activated(int) ), this, SLOT(
                                                      on_merge_combo_activated(int) ) );


    this->merge_label=new QLabel(this);
    this->merge_label->setText("P&ane");
    this->merge_label->setBuddy(this->merge_combo_box);

    this->ui->id_merge_layout->addWidget(this->merge_label);
    this->ui->id_merge_layout->addWidget(this->merge_combo_box);
    this->merge_combo_box->show();


}

int grader_editor::get_merge_index(){
    if(this->merge_combo_box==NULL)
        return 0;
    else
        return this->current_merge_index;
}


void grader_editor::set_combobox_completer(QComboBox * combo_box){
    combo_box->setCompleter(new QCompleter(combo_box));
    combo_box->completer()->setCompletionMode(QCompleter::PopupCompletion);
    combo_box->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    combo_box->completer()->setModel(combo_box->model());
    combo_box->completer()->setFilterMode( Qt::MatchContains );
    combo_box->setValidator(
                new grader_combo_validator( combo_box,combo_box->model() ) );
}


void grader_editor::load_page(bool start,int index){

    if(start || index!=this->current_index ){
        if(!start)
            generate_pdf( false );
        //Will only allow him to go ahed if this page is free of errors
        if(this->tex_compile_errors!=NULL && !start ){
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

            this->ui->file_name_combo->setCurrentIndex(this->current_index);

            load_page_meta_data();
            setup_merge_widget();
            setup_marks_widget();

            this->ui->comment_pos_combo->setCurrentIndex(0);
            this->current_comment_pos_index=0;

            this->ui->comment_text->setText( get_comment() );

            generate_pdf( true );
        }
    }
}


QString grader_editor::get_comment(){
    return this->file_sys_interface->get_comment(
                this->files_list[this->current_index],
this->ui->comment_pos_combo->itemText(this->current_comment_pos_index),get_merge_index() );

}

QString grader_editor::get_marks(){
    return this->file_sys_interface->get_marks(this->files_list[this->current_index],get_merge_index() );

}

void grader_editor::load_page_meta_data(){
    QFile tex_file( this->module_dir_path + "/" + this->files_list[this->current_index] + ".tex" );


    if(!tex_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        display_error( tr( "couldn't open file " ) + this->module_dir_path + "/" +
                            this->files_list[this->current_index] + ".tex" +
                                tr( " for reading page metadata data" ) );
        this->current_marking_scheme="";
        this->current_merge_list=QStringList();
        return;
    }

    QTextStream tex_input_stream(&tex_file);
    QString tex_file_content=tex_input_stream.readAll();
    tex_file.close();

    QRegularExpression marking_scheme_pattern(
       "\\\\showmarkingscheme{([0-9\\." + QString(
                                  marks_denominations_delemiter ) + "]*)}" );
    this->current_marking_scheme = marking_scheme_pattern.match(
                                                tex_file_content).captured(1);


    QRegularExpression merge_pattern(const_merge_pattern);

    QRegularExpressionMatchIterator it=merge_pattern.globalMatch(tex_file_content);

    this->current_merge_list.clear();

    while(it.hasNext()){
        QRegularExpressionMatch merge_pattern_match=it.next();
        this->current_merge_list<<merge_pattern_match.captured(1);
    }

}
