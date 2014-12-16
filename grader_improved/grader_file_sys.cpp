#include "grader_file_sys.h"
#include "constants.h"
#include <QProcess>
#include <QDebug>
#include <QThread>

extern QString latex_compile_command;

grader_file_sys::grader_file_sys(QObject *parent,QString main_tex_dir_name,QString out_dir_name) :
    QObject(parent)
{
    this->main_tex_dir_name=main_tex_dir_name;
    this->out_dir_name=out_dir_name;
}


QString grader_file_sys::get_marks(QString file_name){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    process.setStandardInputFile(this->out_dir_name+"/"+file_name+".tex");
    this->file_mutex.lock();
    process.start("grep", QStringList() << "-oP" << "(?<=putmarks{).*(?=})");
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
//    QString stderr = process.readAllStandardError();
    this->file_mutex.unlock();
    return stdout.simplified();
}

void grader_file_sys::put_marks(QString file_name, QString marks){
    QProcess process;
    marks=marks.simplified();
    process.setWorkingDirectory(this->out_dir_name);
    this->file_mutex.lock();
    process.start("cp", QStringList() << file_name+".tex" << "temp1.tex" );
    process.waitForFinished(-1);
    marks=marks.simplified();
    QString temp="s:\\\\putmarks{.*:\\\\putmarks{"+marks+"}:";
    process.setStandardInputFile(this->out_dir_name+"/temp1.tex");
    process.setStandardOutputFile(this->out_dir_name+"/"+file_name+".tex",QIODevice::Truncate);
    process.start("sed",QStringList() << temp);
    process.waitForFinished(-1);
    process.kill();
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name);
    process1.start("rm temp1.tex");
    process1.waitForFinished(-1);
    this->file_mutex.unlock();
}


QString grader_file_sys::get_comment(QString file_name,QString comment_pos){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    process.setStandardInputFile(this->out_dir_name+"/"+file_name+".tex");
    this->file_mutex.lock();
    QString aa = "(?<=putcomment\\\["+comment_pos+"]{).*(?=})";
    process.start("grep", QStringList() << "-oP" << aa);
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
//    QString stderr = process.readAllStandardError();
    this->file_mutex.unlock();
    return stdout;
}

void grader_file_sys::put_comment(QString file_name, QString comment,QString comment_pos){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->file_mutex.lock();
    process.start("cp", QStringList() << file_name+".tex" << "temp2.tex" );
    process.waitForFinished(-1);
//    comment=comment.simplified();
    comment=escape_string(comment);
    QString temp;
    if(comment_pos=="c")
        temp="s:\\\\customcomment{.*:\\\\customcomment{"+comment+"}:";
    else
        temp="s:\\\\putcomment\\\["+comment_pos+"]{.*:\\\\putcomment\\\["+comment_pos+"]{"+comment+"}:";
    process.setStandardInputFile(this->out_dir_name+"/temp2.tex");
    process.setStandardOutputFile(this->out_dir_name+"/"+file_name+".tex",QIODevice::Truncate);
    process.start("sed",QStringList() << temp);
    process.waitForFinished(-1);
    process.kill();
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name);
    process1.start("rm temp2.tex");
    process1.waitForFinished(-1);
    this->file_mutex.unlock();
}


void grader_file_sys::include_only(bool is_include_only,QString file_name){
    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    process.setStandardInputFile(this->main_tex_dir_name+"/"+const_main_pdf_name+".tex");
    this->main_file_mutex.lock();
    process.start("grep",QStringList() << "\\includeonly{.*");
    process.waitForFinished(-1);
    QProcess process1;
    process1.setWorkingDirectory(this->main_tex_dir_name);
    process1.start("cp "+const_main_pdf_name+".tex temp_pdf.tex");
    process1.waitForFinished(-1);
    process1.setStandardInputFile(this->main_tex_dir_name+"/temp_pdf.tex");
    process1.setStandardOutputFile(this->main_tex_dir_name+"/"+const_main_pdf_name+".tex",QIODevice::Truncate);
    QString temp;
    if(process.readAllStandardOutput()==""){
        if(is_include_only){
            temp="1i\\ \\\\includeonly{"+const_out_dir_name+"\\\/"+file_name+"}";
            process1.start("sed",QStringList()<<temp);
            process1.waitForFinished(-1);
            process1.kill();
        }
    }else{
        if(is_include_only)
            temp="s:\\\\includeonly{.*:\\\\includeonly{"+const_out_dir_name+"\\\/"+file_name+"}:";
        else
            temp="s:\\\\includeonly{.*::";
        process1.start("sed",QStringList()<<temp);
        process1.waitForFinished(-1);
        process1.kill();

    }
    QProcess process2;
    process2.setWorkingDirectory(this->main_tex_dir_name);
    process2.start("rm temp_pdf.tex");
    process2.waitForFinished(-1);
    this->main_file_mutex.unlock();
}


QString grader_file_sys::generate_pdf(QString file_name,QString marks,QString comment_text,QString comment_pos){
    bool is_include_only=true;
    put_marks(file_name,marks);
    put_comment(file_name,comment_text,comment_pos);
    qDebug() << "File sys thread"<<QThread::currentThreadId();
    qDebug()<<"Generate pdf";
    include_only(is_include_only,file_name);
    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    this->tex_mutex.lock();
    process.start(latex_compile_command+" "+const_main_pdf_name+".tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    QString error=tex_error_p2.readAllStandardOutput();
    emit send_error(error);
    return error;
}


QString grader_file_sys::escape_string(QString comment){
    comment=comment.simplified();
    comment.replace("\\","\\\\");
    comment.replace("&","\\&");
    comment.replace(":","\\:");
    QString temp="[\\n\\t\\r]";
//    qDebug() <<temp;
    comment.replace(temp,"+");
//    qDebug() <<comment;
    return comment;
}
