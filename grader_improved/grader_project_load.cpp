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
#include <QTextStream>

#include "constants.h"
#include "grader_combo_validator.h"
#include "grader_project_load.h"
#include "ui_grader_project_load.h"


QChar id_marks_delimiter=';';
QChar marks_denominations_delemiter='+';
QChar settings_delemiter=':';

QString const_bursts_dir_name="bursts";
QString const_main_pdf_name="main_pdf";
QString const_out_dir_name="texfiles";
QString const_sub_tex_name="sub_file.tex";
QString const_top_tex_name="preamble.tex";
QString module_config_file_name="module.config";
QString project_config_file_name="autog.config";
QString settings_file_name="settings.config";
QString latex_compile_command="pdflatex -file-line-error -interaction=nonstopmode";


grader_project_load::grader_project_load(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grader_project_load)
{
    ui->setupUi(this);

    QCompleter *module_name_completer=new QCompleter( this->ui->select_module_combo );
    module_name_completer->setCompletionMode( QCompleter::PopupCompletion );
    module_name_completer->setCaseSensitivity( Qt::CaseInsensitive );
    module_name_completer->setModel( this->ui->select_module_combo->model() );
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
    QDir selected_dir;
    QString dir_path;

    dir_path = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);

    if( dir_path != NULL ){
        this->ui->project_dir_name->setText(dir_path);

        selected_dir=QDir(dir_path);
        if( selected_dir.exists( project_config_file_name ) ){
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
        display_error( tr( "couldn't find " ) + project_location + "/" +
                                                      project_config_file_name );
        return false;
    }

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


QStringList grader_project_load::get_files_list(){
    return this->files_list;
}


QStringList grader_project_load::get_marks_denominations(){
    return this->marks_denominations;
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
                temp_split=temp_line.split(settings_delemiter);
                if( temp_split.size() == 2 && temp_split[1] != NULL)
                    settings_dict.insert( temp_split[0].trimmed(), temp_split[1].trimmed() );
            }
        }

        settings_file.close();

        //changing various settings if provided in settings file
        settings_iterator=settings_dict.find("module_config_file_name");
        if(settings_iterator!=settings_dict.end())
            module_config_file_name=settings_iterator.value();

        settings_iterator=settings_dict.find("const_out_dir_name");
        if(settings_iterator!=settings_dict.end())
            const_out_dir_name=settings_iterator.value();

        settings_iterator=settings_dict.find("const_top_tex_name");
        if(settings_iterator!=settings_dict.end())
            const_top_tex_name=settings_iterator.value();

        settings_iterator=settings_dict.find("const_sub_tex_name");
        if(settings_iterator!=settings_dict.end())
            const_sub_tex_name=settings_iterator.value();

        settings_iterator=settings_dict.find("id_marks_delimiter");
        if(settings_iterator!=settings_dict.end())
            id_marks_delimiter=settings_iterator.value()[0];

        settings_iterator=settings_dict.find("marks_denominations_delemiter");
        if(settings_iterator!=settings_dict.end())
            marks_denominations_delemiter=settings_iterator.value()[0];

        settings_iterator=settings_dict.find("const_main_pdf_name");
        if(settings_iterator!=settings_dict.end())
            const_main_pdf_name=settings_iterator.value();

        settings_iterator=settings_dict.find("const_bursts_dir_name");
        if(settings_iterator!=settings_dict.end())
            const_bursts_dir_name=settings_iterator.value();

        settings_iterator=settings_dict.find("latex_compile_command");
        if(settings_iterator!=settings_dict.end())
            latex_compile_command=settings_iterator.value();

    }


    current_settings+="Current settings are \n";
    current_settings+="settings_delemiter is   \""+QString(settings_delemiter)+"\"\n";
    current_settings+="\"module_config_file_name is   \""+module_config_file_name+"\"\n";
    current_settings+="\"const_out_dir_name is   \""+const_out_dir_name+"\"\n";
    current_settings+="\"const_top_tex_name is   \""+const_top_tex_name+"\"\n";
    current_settings+="\"const_sub_tex_name is   \""+const_sub_tex_name+"\"\n";
    current_settings+="\"id_marks_delimiter is   \""+QString(id_marks_delimiter)+"\"\n";
    current_settings+="\"marks_denominations_delemiter is   \""+QString(marks_denominations_delemiter)+"\"\n";
    current_settings+="\"const_main_pdf_name is   \""+const_main_pdf_name+"\"\n";
    current_settings+="\"const_bursts_dir_name is   \""+const_bursts_dir_name+"\"\n";
    current_settings+="\"latex_compile_command is   \""+latex_compile_command+"\"\n";

    QMessageBox::information(this,tr("Current settings"),current_settings);

}


bool grader_project_load::setup_module(){
    QStringList append_to_main_pdf;
    QString bursts_dir_path=this->project_path+"/"+const_bursts_dir_name;
    QString module_dir_name=this->project_path+"/"+this->module_name;
    QString out_dir_name=module_dir_name+"/"+const_out_dir_name;
    QString sub_tex_path=this->project_path+"/"+const_sub_tex_name;
    QString top_tex_path=this->project_path+"/"+const_top_tex_name;
    QDir project_dir(this->project_path);
    QDir module_dir(module_dir_name);//this exists as checked in start btn slot
    QDir out_dir(out_dir_name);
    QFile module_config_file(module_dir_name+"/"+module_config_file_name);
    QFile main_pdf_tex_file(module_dir_name+"/"+const_main_pdf_name+".tex");
    QRegularExpression burst_path_pattern("\\\\newcommand{\\\\burstsdir}{.*");
    QRegularExpression put_page_pattern("\\\\putpage{.*");


    if(!project_dir.exists(const_top_tex_name)){
        display_error( tr( "couldn't find preamble file " ) + this->project_path +
                                                        "/" + const_top_tex_name );
        return false;
    }
    if(!project_dir.exists(const_sub_tex_name)){
        display_error( tr( "couldn't find sub file " ) + this->project_path +
                                                        "/" + const_sub_tex_name );
        return false;
    }

    if(!project_dir.exists(const_bursts_dir_name)){
        display_error( tr( "couldn't find bursts dir " ) + this->project_path +
                                                    "/" + const_bursts_dir_name );
        return false;
    }

    if( !( module_dir.exists(const_out_dir_name) ||
                                        module_dir.mkdir( const_out_dir_name ) ) ){
        display_error( tr( "couldn't create output dir " ) +module_dir_name +
                                                               "/" + out_dir_name );
        return false;
    }



    if( ! module_config_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        display_error( tr( "couldn't open module config file " ) + module_dir_name +
                                                    "/" + module_config_file_name );
        return false;
    }


    QTextStream module_config_file_stream(&module_config_file);
    QString temp_line;
    QStringList temp_split;

    while( ! module_config_file_stream.atEnd() ){
        temp_line=module_config_file_stream.readLine();
        if( temp_line != NULL ){
            temp_split=temp_line.split( id_marks_delimiter );
            this->files_list<<temp_split[0];
            this->marks_denominations<<temp_split[1];
        }
    }

    module_config_file.close();

    if(this->files_list.length()==0){
        display_error( tr( "No id's in the module config file " ) + module_dir_name +
                                                      "/" + module_config_file_name );
        return false;
    }

    //creating the sub texfiles
    for( int i=0; i < this->files_list.length(); i++ ){
        if( ! out_dir.exists( out_dir_name + "/" + this->files_list[i] + ".tex" ) ){
            if( ! QFile::copy( sub_tex_path , out_dir_name + "/" +
                                                    this->files_list[i] + ".tex") ){

                display_error( tr( "couldn't copy file from " ) + sub_tex_path +
                                    tr(" to ") + out_dir_name + "/" +
                                                        this->files_list[i] + ".tex" );
                return false;
            }
        }
    }


    //creating main pdf tex file
    if( module_dir.exists( const_main_pdf_name + ".tex" ) ){
        if( ! module_dir.remove( const_main_pdf_name + ".tex" ) ){
            display_error( tr( "couldn't file " ) + module_dir_name + "/" +
                           const_main_pdf_name + ".tex" +
                          tr( " exists and couldn't be removed for overwriting" ) );
            return false;
        }
    }

    if( ! QFile::copy( top_tex_path , module_dir_name + "/" + const_main_pdf_name +
                                                                            ".tex" ) ){
        display_error( tr( "couldn't copy file from " ) + top_tex_path + tr( " to " ) +
                                module_dir_name + "/" + const_main_pdf_name + ".tex" );
        return false;
    }



    append_to_main_pdf<<"\\begin{document}";
    for(int i=0;i<this->files_list.length();i++){
        append_to_main_pdf<<"\\include{"+const_out_dir_name+"/"+this->files_list[i]+"}";
    }
    append_to_main_pdf<< "\\end{document}";


    if( ! main_pdf_tex_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
        display_error( tr( "couldn't open file " ) + module_dir_name +
                            "/" + const_main_pdf_name + ".tex" + tr( " for read" ) );
        return false;
    }



    //Adding th burst dir path to main pdf tex
    QTextStream main_pdf_tex_input_stream(&main_pdf_tex_file);
    QString main_pdf_tex_content=main_pdf_tex_input_stream.readAll();
    main_pdf_tex_file.close();


    if( ! main_pdf_tex_file.open( QIODevice::WriteOnly | QIODevice::Text |
                                                               QIODevice::Truncate ) ){
        display_error( tr( "couldn't open file " ) + module_dir_name + "/" +
                                    const_main_pdf_name + ".tex" + tr( " for write" ) );
        return false;
    }


    QTextStream main_pdf_tex_output_stream(&main_pdf_tex_file);
    main_pdf_tex_content.replace(burst_path_pattern,"\\newcommand{\\burstsdir}{"+bursts_dir_path+"}");
    main_pdf_tex_output_stream<<main_pdf_tex_content<<"\n";
    //const reference makes it faster as we are not creating any new object
    foreach(const QString &temp_line,append_to_main_pdf)
        main_pdf_tex_output_stream<<temp_line<<"\n";
    main_pdf_tex_file.flush();
    main_pdf_tex_file.close();


    //Inserting putpage value into the sub tex files
    for( int i=0; i < this->files_list.length(); i++ ){
        QFile sub_tex_file( out_dir_name + "/" + this->files_list[i] + ".tex" );

        if(!sub_tex_file.open( QIODevice::ReadOnly | QIODevice::Text ) ){
            display_error( tr( "couldn't open file " ) + out_dir_name + "/" +
                                this->files_list[i] + ".tex" + tr( " for read" ) );
            return false;
        }

        QTextStream sub_tex_input_stream(&sub_tex_file);
        QString sub_tex_file_content=sub_tex_input_stream.readAll();
        sub_tex_file.close();

        if( ! sub_tex_file.open( QIODevice::WriteOnly | QIODevice::Text |
                                                            QIODevice::Truncate ) ){
            display_error(tr("couldn't open file ") + out_dir_name + "/" +
                                this->files_list[i] + ".tex" + tr( " for write" ) );
            return false;
        }

        QTextStream sub_tex_output_stream(&sub_tex_file);
        sub_tex_file_content.replace(put_page_pattern,"\\putpage{"+this->files_list[i]+"}");
        sub_tex_output_stream<<sub_tex_file_content;

        sub_tex_file.flush();
        sub_tex_file.close();
    }
    return true;
}


void grader_project_load::display_error(QString error){
    QMessageBox::critical(this,tr("Error"),error);
}
