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
#include <QDir>
#include <QDesktopServices>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>
#include <QThread>
#include <QUrl>

#include "constants.h"
#include "grader_file_sys.h"


grader_file_sys::grader_file_sys(QObject *parent,QString module_dir_path) :
    QObject(parent)
{
    this->module_dir_path=module_dir_path;
}


QString grader_file_sys::get_marks(QString file_name){
    QFile sub_tex_file(this->module_dir_path+"/"+file_name+".tex");
    QString marks=QString();
    this->sub_tex_files_edit_lock.lock();
    if(sub_tex_file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QTextStream sub_tex_stream(&sub_tex_file);
        QString sub_tex_content=sub_tex_stream.readAll();
        sub_tex_file.close();
        this->sub_tex_files_edit_lock.unlock();
        QRegularExpression get_marks_pattern("\\\\putmarks{([^}]*)}");
        marks=get_marks_pattern.match(sub_tex_content).captured(1);
    }else{
        this->sub_tex_files_edit_lock.unlock();
        emit send_error(tr("couldn't open file ")+this->module_dir_path+"/"+file_name+".tex"+tr(" to read marks"));
    }
    return marks.simplified();
}

void grader_file_sys::put_marks(QString file_name, QString marks){
    QFile file(this->module_dir_path+"/"+file_name+".tex");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't Open ")+this->module_dir_path+"/"+file_name+".tex" +tr("for reading marks"));
        return ;
    }
    QTextStream input(&file);
    QString content=input.readAll();
    file.close();
    QRegularExpression pattern;
    QString replacement;
    pattern=QRegularExpression("\\\\putmarks{[^}]*}");
    replacement="\\putmarks{"+marks.simplified()+"}";
    content.replace(pattern,replacement);
    this->sub_tex_files_edit_lock.lock();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        this->sub_tex_files_edit_lock.unlock();
        emit send_error(tr("couldn't Open ")+this->module_dir_path+"/"+file_name+".tex"+tr("for writing marks"));
        return ;
    }
    QTextStream input1(&file);
    input1<<content;
    file.flush();
    file.close();
    this->sub_tex_files_edit_lock.unlock();
}

void grader_file_sys::put_comment(QString file_name, QString comment,QString comment_pos){
    QFile file(this->module_dir_path+"/"+file_name+".tex");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't Open ")+this->module_dir_path+"/"+file_name+".tex"+tr("for reading comment"));
        return ;
    }
    QTextStream input(&file);
    QString content=input.readAll();
    file.close();
    QRegularExpression pattern;
    QString replacement;

    //(?!foo) is negative lookahed
    pattern=QRegularExpression("\\\\putcomment"+comment_pos+"{((?!}\\\\nextcommandmarker).)*}\\\\nextcommandmarker",QRegularExpression::DotMatchesEverythingOption);
    replacement="\\putcomment"+comment_pos+"{"+comment+"}\\nextcommandmarker";
    content.replace(pattern,replacement);

    this->sub_tex_files_edit_lock.lock();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        emit send_error(tr("couldn't Open ")+this->module_dir_path+"/"+file_name+".tex"+tr("for writing comment"));
        this->sub_tex_files_edit_lock.unlock();
        return ;
    }
    QTextStream input1(&file);
    input1<<content;
    file.flush();
    file.close();
    this->sub_tex_files_edit_lock.unlock();
}

QString grader_file_sys::get_comment(QString file_name,QString comment_pos){
    QFile file(this->module_dir_path+"/"+file_name+".tex");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't Open ")+this->module_dir_path+"/"+file_name+".tex"+tr("for reading comment"));
        return QString();
    }
    QTextStream input(&file);
    QString content=input.readAll();
    file.close();
    QRegularExpression pattern;

    //(?!foo) is negative lookahed
    pattern=QRegularExpression("\\\\putcomment"+comment_pos+"{(((?!}\\\\nextcommandmarker).)*)}\\\\nextcommandmarker",QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatch comment_match=pattern.match(content);
    return comment_match.captured(1);
}


QString grader_file_sys::generate_pdf(QString file_name,QString marks,QString comment_text,QString comment_pos){
    put_marks(file_name,marks);
    put_comment(file_name,comment_text,comment_pos);

    QProcess process;
    process.setWorkingDirectory(this->module_dir_path);
    this->tex_compile_lock.lock();
    process.start(latex_compile_command+" "+file_name+".tex");
    process.waitForFinished(-1);
    this->tex_compile_lock.unlock();
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    QRegularExpression error_pattern(".+:[0-9]+:.+|^l\\.[0-9]+.*",QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator error_iterator=error_pattern.globalMatch(temp_tex_errors);
    QString error;
    while (error_iterator.hasNext()) {
        QRegularExpressionMatch match = error_iterator.next();
        error=error+ match.captured(0)+"\n";
    }
    if( error != NULL ){
        error=tr( "Compile command :\n")+latex_compile_command+
                " "+file_name+".tex"+ "\n\n" + tr(" Errors :")+"\n" + error;
    }

    emit send_tex_compile_error(error);

    if( error == NULL ){
        qDebug()<<"Null error";
        qDebug()<<temp_tex_errors;
        if( QFile::exists(this->module_dir_path + "/" +
                          const_build_dir_name + "/" +
                                        const_main_pdf_name + ".pdf" ) ){
            if ( ! QFile::remove(this->module_dir_path + "/" +
                          const_build_dir_name + "/" +
                                        const_main_pdf_name + ".pdf")){

                emit send_error( this->module_dir_path + "/" +
                                 const_build_dir_name + "/" +
                                      const_main_pdf_name + ".pdf" +
                           tr( " exists and couldn't be removed for overwriting" ) );

            }
        }
        if( QFile::exists(this->module_dir_path + "/" +
                                    const_build_dir_name + "/" +
                                                        file_name + ".pdf") ){
            if( ! QFile::copy(this->module_dir_path + "/" +
                            const_build_dir_name + "/" +
                              file_name + ".pdf", this->module_dir_path +
                            "/" + const_build_dir_name + "/" +
                                 const_main_pdf_name + ".pdf" ) ){

                emit send_error( tr("Couldn't copy file from ") +
                                 this->module_dir_path + "/" +
                                    const_build_dir_name + "/" +
                                      file_name + ".pdf"+
                                    tr(" to ") + this->module_dir_path +
                                 "/" + const_build_dir_name + "/" +
                                      const_main_pdf_name + ".pdf");
            }
        }else{
            emit send_error( tr("file ") +
                             this->module_dir_path + "/" +
                                const_build_dir_name + "/" +
                                  file_name + ".pdf"+ tr(" dosen't exist"));
        }
    }

    return error;
}


void grader_file_sys::open_pdf(){
    if( ! QDesktopServices::openUrl( QUrl( "file:///" + this->module_dir_path +
                                       "/" + const_build_dir_name + "/" +
                                                const_main_pdf_name + ".pdf",
                                                        QUrl::TolerantMode ) ) ){

        emit send_error( tr( "couldn't open file " ) + this->module_dir_path +
                                        "/" + const_build_dir_name + "/" +
                                                    const_main_pdf_name + ".pdf");
    }
}


void grader_file_sys::open_tex_file(QString file_name){
    if( ! QDesktopServices::openUrl( QUrl( "file:///" +
                                  this->module_dir_path + "/" +
                                        file_name +
                                                ".tex", QUrl::TolerantMode ) ) ){

        emit send_error( tr( "couldn't open file " ) + this->module_dir_path +
                            "/" + file_name + ".tex" );

    }
}
