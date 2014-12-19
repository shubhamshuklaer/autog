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
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>
#include <QThread>

#include "constants.h"
#include "grader_file_sys.h"


grader_file_sys::grader_file_sys(QObject *parent,QString main_tex_dir_name,QString out_dir_name,QString sub_tex_path) :
    QObject(parent)
{
    this->main_tex_dir_name=main_tex_dir_name;
    this->out_dir_name=out_dir_name;
    this->sub_tex_path=sub_tex_path;
}


QString grader_file_sys::get_marks(QString file_name){
    QFile sub_tex_file(this->out_dir_name+"/"+file_name+".tex");
    QString marks=QString();
    this->file_mutex.lock();
    if(sub_tex_file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QTextStream sub_tex_stream(&sub_tex_file);
        QString sub_tex_content=sub_tex_stream.readAll();
        sub_tex_file.close();
        this->file_mutex.unlock();
        QRegularExpression get_marks_pattern("\\\\putmarks{([^}]*)}");
        marks=get_marks_pattern.match(sub_tex_content).captured(1);
    }else{
        this->file_mutex.unlock();
        emit send_error(tr("couldn't open file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" to read marks"));
//        QMessageBox::warning(qobject_cast<QWidget *> (this->parent()),tr("Error"),tr("couldn't open file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" to read marks"));
    }
    return marks.simplified();
}

void grader_file_sys::put_marks(QString file_name, QString marks){
    QFile file(this->out_dir_name+"/"+file_name+".tex");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex" +tr("for reading marks"));
//        QMessageBox::warning(
//                    qobject_cast<QWidget *> (this->parent()),
//                    tr("Grader"),
//                    tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex" +tr("for reading marks"));
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
    this->file_mutex.lock();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        this->file_mutex.unlock();
        emit send_error(tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for writing marks"));
//        QMessageBox::warning(
//                    qobject_cast<QWidget *> (this->parent()),
//                    tr("Grader"),
//                    tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for writing marks"));
        return ;
    }
    QTextStream input1(&file);
    input1<<content;
    file.flush();
    file.close();
    this->file_mutex.unlock();
}

void grader_file_sys::put_comment(QString file_name, QString comment,QString comment_pos){
    QFile file(this->out_dir_name+"/"+file_name+".tex");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for reading comment"));
//        QMessageBox::warning(
//                    qobject_cast<QWidget *> (this->parent()),
//                    tr("Grader"),
//                    tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for reading comment"));
        return ;
    }
    QTextStream input(&file);
    QString content=input.readAll();
    file.close();
    QRegularExpression pattern;
    QString replacement;
    if(comment_pos=="c"){
        //(?!foo) is negative lookahed
        pattern=QRegularExpression("\\\\customcomment{((?!}\\\\nextcommandmarker).)*}\\\\nextcommandmarker",QRegularExpression::DotMatchesEverythingOption);
        replacement="\\customcomment{"+comment+"}\\nextcommandmarker";
    }else{
        //(?!foo) is negative lookahed
        pattern=QRegularExpression("\\\\putcomment\\["+comment_pos+"]{((?!}\\\\nextcommandmarker).)*}\\\\nextcommandmarker",QRegularExpression::DotMatchesEverythingOption);
        replacement="\\putcomment["+comment_pos+"]{"+comment+"}\\nextcommandmarker";
    }
    content.replace(pattern,replacement);
    this->file_mutex.lock();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        emit send_error(tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for writing comment"));
//        QMessageBox::warning(
//                    qobject_cast<QWidget *> (this->parent()),
//                    tr("Grader"),
//                    tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for writing comment"));
        this->file_mutex.unlock();
        return ;
    }
    QTextStream input1(&file);
    input1<<content;
    file.flush();
    file.close();
    this->file_mutex.unlock();
}

QString grader_file_sys::get_comment(QString file_name,QString comment_pos){
    QFile file(this->out_dir_name+"/"+file_name+".tex");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for reading comment"));
//        QMessageBox::warning(
//                    qobject_cast<QWidget *> (this->parent()),
//                    tr("Grader"),
//                    tr("couldn't Open ")+this->out_dir_name+"/"+file_name+".tex"+tr("for reading comment"));
        return QString();
    }
    QTextStream input(&file);
    QString content=input.readAll();
    file.close();
    QRegularExpression pattern;
    if(comment_pos=="c"){
        //(?!foo) is negative lookahed
        pattern=QRegularExpression("\\\\customcomment{(((?!}\\\\nextcommandmarker).)*)}\\\\nextcommandmarker",QRegularExpression::DotMatchesEverythingOption);
    }else{
        //(?!foo) is negative lookahed
        pattern=QRegularExpression("\\\\putcomment\\["+comment_pos+"]{(((?!}\\\\nextcommandmarker).)*)}\\\\nextcommandmarker",QRegularExpression::DotMatchesEverythingOption);
    }
    QRegularExpressionMatch comment_match=pattern.match(content);
    return comment_match.captured(1);
}



bool grader_file_sys::include_only(bool is_include_only,QString file_name){
    this->main_file_mutex.lock();
    QFile main_pdf_tex_file(this->main_tex_dir_name+"/"+const_main_pdf_name+".tex");
    if(!main_pdf_tex_file.open(QIODevice::ReadOnly|QIODevice::Text)){
        this->main_file_mutex.unlock();
        emit send_error(tr("couldn't open file ")+this->main_tex_dir_name+"/"+const_main_pdf_name+".tex"+tr(" to read"));
//        QMessageBox::warning(qobject_cast<QWidget *> (this->parent()),tr("Error"),tr("couldn't open file ")+this->main_tex_dir_name+"/"+const_main_pdf_name+".tex"+tr(" to read"));
        return false;
    }
    QTextStream main_pdf_tex_input_stream(&main_pdf_tex_file);
    QString main_pdf_tex_content=main_pdf_tex_input_stream.readAll();
    main_pdf_tex_file.close();
    QRegularExpression include_only_pattern("\\\\includeonly{.*");
    QString replacement;
    if(!main_pdf_tex_content.contains(include_only_pattern)){
        if(is_include_only){
            replacement="\\includeonly{"+const_out_dir_name+"/"+file_name+"}";
            main_pdf_tex_content=replacement+"\n"+main_pdf_tex_content;
        }
    }else{
        if(is_include_only){
            main_pdf_tex_content.replace(include_only_pattern,"\\includeonly{"+const_out_dir_name+"/"+file_name+"}");
        }else{
            main_pdf_tex_content.replace(include_only_pattern,"\\includeonly{"+const_out_dir_name+"/"+file_name+"}");
        }
    }

    if(!main_pdf_tex_file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)){
        this->main_file_mutex.unlock();
        emit send_error(tr("couldn't open file ")+this->main_tex_dir_name+"/"+const_main_pdf_name+".tex"+tr(" to write"));
        //        QMessageBox::warning(qobject_cast<QWidget *> (this->parent()),tr("Error"),tr("couldn't open file ")+this->main_tex_dir_name+"/"+const_main_pdf_name+".tex"+tr(" to write"));
        return false;
    }

    QTextStream main_pdf_tex_output_stream(&main_pdf_tex_file);
    main_pdf_tex_output_stream<<main_pdf_tex_content;
    main_pdf_tex_file.flush();
    main_pdf_tex_file.close();
    this->main_file_mutex.unlock();
    return true;
}



QString grader_file_sys::generate_pdf(QString file_name,QString marks,QString comment_text,QString comment_pos){
    bool is_include_only=true;
    put_marks(file_name,marks);
    put_comment(file_name,comment_text,comment_pos);
    include_only(is_include_only,file_name);


    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    this->tex_mutex.lock();
    process.start(latex_compile_command+" "+const_main_pdf_name+".tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    QRegularExpression error_pattern(".+:[0-9]+:.+|^l\\.[0-9]+.*",QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator error_iterator=error_pattern.globalMatch(temp_tex_errors);
    QString error;
    while (error_iterator.hasNext()) {
        QRegularExpressionMatch match = error_iterator.next();
        error=error+ match.captured(0)+"\n";
    }
    emit send_tex_error(error);
    return error;
}



void grader_file_sys::fix_file(QString file_name){
    QDir out_dir(this->out_dir_name);
    if(out_dir.exists(file_name+".tex")){
        if(!out_dir.remove(file_name+".tex")){
            emit send_error(tr("couldn't file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" exists and couldn't be removed for overwriting"));
//            QMessageBox::warning(
//                        qobject_cast<QWidget *> (this->parent()),
//                        tr("Error"),
//                        tr("couldn't file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" exists and couldn't be removed for overwriting"));
            return;
        }
    }

    if(!QFile::copy(this->sub_tex_path,this->out_dir_name+"/"+file_name+".tex")){
        emit send_error(tr("couldn't copy file from ")+this->sub_tex_path+tr(" to ")+this->out_dir_name+"/"+file_name+".tex");
//        QMessageBox::warning(
//                    qobject_cast<QWidget *> (this->parent()),
//                    tr("Error"),
//                    tr("couldn't copy file from ")+this->sub_tex_path+tr(" to ")+this->out_dir_name+"/"+file_name+".tex");
        return;
    }
    //putting put page

    QFile sub_tex_file(this->out_dir_name+"/"+file_name+".tex");

    if(!sub_tex_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit send_error(tr("couldn't open file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" for read"));
//        QMessageBox::warning(qobject_cast<QWidget *> (this->parent()),tr("Error"),tr("couldn't open file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" for read"));
        return;
    }

    QTextStream sub_tex_input_stream(&sub_tex_file);
    QString sub_tex_content=sub_tex_input_stream.readAll();
    sub_tex_file.close();

    if(!sub_tex_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        emit send_error(tr("couldn't open file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" for read"));
//        QMessageBox::warning(qobject_cast<QWidget *> (this->parent()),tr("Error"),tr("couldn't open file ")+this->out_dir_name+"/"+file_name+".tex"+tr(" for read"));
        return;
    }


    QTextStream sub_tex_output_stream(&sub_tex_file);
    QRegularExpression put_page_pattern("\\\\putpage{.*");
    qDebug()<<"Sub tex content"<<sub_tex_content;
    sub_tex_content.replace(put_page_pattern,"\\putpage{"+file_name+"}");
    qDebug()<<sub_tex_content;
    sub_tex_output_stream<<sub_tex_content;

    sub_tex_file.flush();
    sub_tex_file.close();
}
