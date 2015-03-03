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
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHash>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>

#include "constants.h"
#include "grader_combo_validator.h"
#include "grader_project_load.h"
#include "ui_grader_project_load.h"


grader_project_load::grader_project_load(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grader_project_load)
{
    ui->setupUi(this);

    QCompleter *module_name_completer=new QCompleter( this->ui->select_module_combo );
    module_name_completer->setCompletionMode( QCompleter::PopupCompletion );
    module_name_completer->setCaseSensitivity( Qt::CaseInsensitive );
    module_name_completer->setModel( this->ui->select_module_combo->model() );
    module_name_completer->setFilterMode( Qt::MatchContains );
    this->ui->select_module_combo->setCompleter( module_name_completer );

    this->ui->select_module_combo->setValidator( new
            grader_combo_validator( this, this->ui->select_module_combo->model() ) );

    connect( this->ui->select_project_btn, SIGNAL( clicked() ), this,
                                              SLOT( select_project_btn_clicked() ) );
    connect( this->ui->start_btn, SIGNAL( clicked() ), this,
                                                       SLOT( start_btn_clicked() ) );
}


grader_project_load::~grader_project_load(){
    delete ui;
}


void grader_project_load::select_project_btn_clicked(){
    QSettings project_settings("project_settings");
    QDir selected_dir;
    QString dir_path,previous_path;

    previous_path=project_settings.value("previous_path").toString();

    if(previous_path=="")
        previous_path="/home";

    dir_path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 previous_path,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if( dir_path != NULL ){

        selected_dir=QDir(dir_path);
        project_settings.setValue("previous_path",dir_path);

        if( selected_dir.exists( project_config_file_name ) ){
            this->ui->project_dir_path->setText(dir_path);
            if( configure_project( dir_path ) ){
                this->ui->select_module_widget->setEnabled( true );
                load_settings();
            }
        }else{
            display_error( tr("Not a valid autog project, couldn't find ") +
                                         dir_path + "/" + project_config_file_name);
        }
    }
}


void grader_project_load::start_btn_clicked()
{
    QString temp_module_name;
    QDir project_dir(this->project_path);

    if(this->ui->select_module_combo->currentIndex()!=-1){
        temp_module_name=this->ui->select_module_combo->itemText(
                                    this->ui->select_module_combo->currentIndex());

        if(project_dir.exists(temp_module_name)){
            this->module_name=temp_module_name;
            if( setup_module() )
                emit done();
        }else{
            display_error( tr( "module directory named " ) + this->project_path +
                                 "/" + temp_module_name+ tr( " doens't exist" ) );
        }
    }else{
        display_error( tr( "Please select a module" ) );
    }
}


bool grader_project_load::configure_project(QString project_location){
    QFile project_config_file( project_location+"/"+project_config_file_name );;
    QTextStream config_text_stream( &project_config_file );


    if ( ! project_config_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        display_error( tr( "couldn't open " ) + project_location + "/" +
                                                      project_config_file_name );
        return false;
    }

    //to clear list before adding new items
    this->ui->select_module_combo->clear();
    while(!config_text_stream.atEnd()){
        this->ui->select_module_combo->addItem(config_text_stream.readLine());
    }

    project_config_file.close();

    this->project_path=project_location;

    return true;
}


QString grader_project_load::get_module_name(){
    return this->module_name;
}


QString grader_project_load::get_project_path(){
    return this->project_path;
}

int grader_project_load::get_grading_start(){
    return this->start_grading_from;
}

QStringList grader_project_load::get_files_list(){
    return this->files_list;
}


QStringList grader_project_load::get_marks_denominations_list(){
    return this->marks_denominations_list;
}

QList<QStringList> grader_project_load::get_merge_list(){
    return this->merge_list;
}

void grader_project_load::load_settings(){
    QFile settings_file(this->project_path+"/"+settings_file_name);
    QDir project_dir(this->project_path);
    QHash<QString,QString> settings_dict;
    QHash<QString,QString>::iterator settings_iterator;
    QString current_settings;

    if( project_dir.exists( settings_file_name ) ){

        if( ! settings_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
            display_error( tr( "settings config file exists but couldn't open " ) +
                                      this->project_path + "/" + settings_file_name );
            return;
        }


        QTextStream settings_file_stream(&settings_file);
        QString temp_line;
        QStringList temp_split;

        //check to see if settings_delemiter is provided in settings file
        if(!settings_file_stream.atEnd()){
            temp_line=settings_file_stream.readLine();
            if(temp_line.trimmed().length()==1){
                settings_delemiter=temp_line.trimmed()[0];
            }else{
                settings_file_stream.seek(0);
            }
        }

        while(!settings_file_stream.atEnd()){
            temp_line=settings_file_stream.readLine().trimmed();
            if(temp_line!=NULL){
                if(temp_line[0]!=const_comment_char){//ignoring comment lines
                    temp_split=temp_line.split(settings_delemiter);
                    if( temp_split.size() == 2 && temp_split[1] != NULL)
                        settings_dict.insert( temp_split[0].trimmed(), temp_split[1].trimmed() );
                }
            }
        }

        settings_file.close();

        //changing various settings if provided in settings file
        settings_iterator=settings_dict.find("module_config_file_name");
        if(settings_iterator!=settings_dict.end())
            module_config_file_name=settings_iterator.value();


        settings_iterator=settings_dict.find("marks_denominations_delemiter");
        if(settings_iterator!=settings_dict.end())
            marks_denominations_delemiter=settings_iterator.value()[0];

        settings_iterator=settings_dict.find("const_main_pdf_name");
        if(settings_iterator!=settings_dict.end())
            const_main_pdf_name=settings_iterator.value();

        settings_iterator=settings_dict.find("const_build_dir_name");
        if(settings_iterator!=settings_dict.end())
            const_build_dir_name=settings_iterator.value();

        settings_iterator=settings_dict.find("latex_compile_command");
        if(settings_iterator!=settings_dict.end())
            latex_compile_command=settings_iterator.value();

        settings_iterator=settings_dict.find("const_tex_compile_timeout");
        if(settings_iterator!=settings_dict.end())
            const_tex_compile_timeout=settings_iterator.value().toInt();

    }


    current_settings+="Current settings are \n";
    current_settings+="settings_delemiter is   \""+QString(settings_delemiter)+"\"\n";
    current_settings+="\"module_config_file_name is   \""+module_config_file_name+"\"\n";
    current_settings+="\"marks_denominations_delemiter is   \""+QString(marks_denominations_delemiter)+"\"\n";
    current_settings+="\"const_main_pdf_name is   \""+const_main_pdf_name+"\"\n";
    current_settings+="\"const_build_dir_name is   \""+const_build_dir_name+"\"\n";
    current_settings+="\"latex_compile_command is   \""+latex_compile_command+"\"\n";
    current_settings+="\"const_tex_compile_timeout is   \""+QString::number(const_tex_compile_timeout)+tr(" msecs")+"\"\n";

    QMessageBox::information(this,tr("Current settings"),current_settings);

}


void grader_project_load::display_error(QString error){
    QMessageBox::critical(this,tr("Error"),error);
}


bool grader_project_load::setup_module(){
    QString module_dir_path=this->project_path+"/"+this->module_name;
    QDir module_dir(module_dir_path);
    QFile module_config_file(module_dir_path+"/"+module_config_file_name);

    if( ! module_dir.exists( module_dir_path+"/"+module_config_file_name ) ){
        display_error( tr( "Module config file ") + module_dir_path +
                       "/" + module_config_file_name + tr( " not found" ) );
        return false;
    }


    if( ! module_config_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        display_error( tr( "couldn't open module config file " ) + module_dir_path +
                                                            "/" + module_config_file_name );
                return false;
    }

    QTextStream module_config_file_stream( &module_config_file );
    QString temp_line;
    while( ! module_config_file_stream.atEnd() ){
        temp_line=module_config_file_stream.readLine();
        if( temp_line != NULL ){
            this->files_list<<temp_line;
        }
    }

    module_config_file.close();

    if( this->files_list.length() == 0 ){
        display_error( tr( "No entries in file " ) +
                       module_dir_path + "/" + module_config_file_name );
        return false;
    }

    bool got_grading_start=false;
    this->start_grading_from=-1;

    foreach( const QString & file_name, this->files_list ){
        QFile tex_file( module_dir_path + "/" + file_name + ".tex" );

        if(!tex_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
            display_error( tr( "couldn't open file " ) + module_dir_path + "/" +
                                file_name + ".tex" +
                                    tr( " for reading marking scheme" ) );
            return false;
        }

        QTextStream tex_input_stream(&tex_file);
        QString tex_file_content=tex_input_stream.readAll();
        tex_file.close();

        QRegularExpression marking_scheme_pattern(
           "\\\\showmarkingscheme{([0-9\\." + QString(
                                      marks_denominations_delemiter ) + "]*)}" );
        this->marks_denominations_list << marking_scheme_pattern.match(
                                                    tex_file_content).captured(1);


        QRegularExpression merge_pattern(const_merge_pattern);

        QRegularExpressionMatchIterator it=merge_pattern.globalMatch(tex_file_content);

        QStringList temp_merge_list;
        while(it.hasNext()){
            QRegularExpressionMatch merge_pattern_match=it.next();
            temp_merge_list<<merge_pattern_match.captured(1);
        }

        this->merge_list<<temp_merge_list;

        if( !got_grading_start ){
            QRegularExpression put_marks_pattern("\\\\putmarks{(.*)}");
            if( put_marks_pattern.match( tex_file_content ).captured(1) == NULL ){
                got_grading_start=true;
            }
            start_grading_from++;
        }

    }

    if( !got_grading_start )
        start_grading_from=0;

    return true;
}
