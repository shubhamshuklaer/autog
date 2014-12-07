#include "grader_editor.h"
#include "ui_grader_editor.h"
#include <QProcess>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QCompleter>
#include "grader_marks_widget.h"

grader_editor::grader_editor(QWidget *parent,QStringList filesList,QString out_dir_name,QString sub_tex_name) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);
    this->filesList=filesList;
    this->out_dir_name=out_dir_name;
    this->current_index=0;
    this->sub_tex_name=sub_tex_name;
    this->ui->file_name_combo->addItems(this->filesList);
    this->ui->file_name_combo->setCurrentIndex(0);
    QStringList marks_denominations=QStringList()<<"0.5"<<"0.5"<<"1"<<"2";
    this->marks_widget=new grader_marks_widget(this->ui->marks_widget,marks_denominations);
//    t->show();
    //    this->ui->marks_widget->layout()->addWidget(temp);
    this->marks_widget->setProperty("marks",get_marks(this->filesList[0]));
    this->ui->comment_text->setText(get_comment(this->filesList[0]));
    if(this->current_index+1>=this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(false);
    connect(this->marks_widget,SIGNAL(marks_changed()),this,SLOT(on_marks_text_textChanged()));

    this->ui->marks_label->setBuddy(this->marks_widget);

    this->ui->file_name_combo->setCompleter(new QCompleter(this->ui->file_name_combo));
    this->ui->file_name_combo->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    this->ui->file_name_combo->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    this->ui->file_name_combo->completer()->setModel(this->ui->file_name_combo->model());


    this->ui->comment_pos_combo->setCompleter(new QCompleter(this->ui->comment_pos_combo));
    this->ui->comment_pos_combo->completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    this->ui->comment_pos_combo->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    this->ui->comment_pos_combo->completer()->setModel(this->ui->comment_pos_combo->model());
}

grader_editor::~grader_editor()
{
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    if(this->current_index+2==this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(true);
    this->current_index++;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    this->marks_widget->setProperty("marks",get_marks(this->filesList[this->current_index]));
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index]));

}

void grader_editor::on_prev_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    if(this->current_index<=1)
        this->ui->prev_btn->setEnabled(false);
    this->ui->next_btn->setEnabled(true);
    this->current_index--;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    this->marks_widget->setProperty("marks",get_marks(this->filesList[this->current_index]));
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index]));
}

void grader_editor::on_preview_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -file-line-error -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    tex_error_p1.setWorkingDirectory(this->out_dir_name);
    tex_error_p2.setWorkingDirectory(this->out_dir_name);
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    this->tex_errors_lock.lockForWrite();
    this->tex_errors=tex_error_p2.readAllStandardOutput();
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
    process.start("gnome-open", QStringList() << "main_pdf.pdf");
    process.waitForFinished(-1);
}

void grader_editor::on_gen_pdf_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(false);
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -file-line-error -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    tex_error_p1.setWorkingDirectory(this->out_dir_name);
    tex_error_p2.setWorkingDirectory(this->out_dir_name);
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    this->tex_errors_lock.lockForWrite();
    this->tex_errors=tex_error_p2.readAllStandardOutput();
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
    process.start("gnome-open", QStringList() << "main_pdf.pdf");
    process.waitForFinished(-1);
}

void grader_editor::on_file_name_combo_activated(int index)
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    this->current_index=index;
    if(this->current_index==0)
        this->ui->prev_btn->setEnabled(false);
    else
        this->ui->prev_btn->setEnabled(true);
    if(this->current_index+1>=this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    else
        this->ui->next_btn->setEnabled(true);
    this->marks_widget->setProperty("marks",get_marks(this->filesList[this->current_index]));
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index]));

}



QString grader_editor::get_marks(QString file_name){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process.setStandardInputFile(this->out_dir_name+"/subfiles/"+file_name+".tex");
    this->file_mutex.lock();
    process.start("grep", QStringList() << "-oP" << "(?<=putmarks{).*(?=})");
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
//    QString stderr = process.readAllStandardError();
    this->file_mutex.unlock();
    return stdout;
}

QString grader_editor::get_comment(QString file_name){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process.setStandardInputFile(this->out_dir_name+"/subfiles/"+file_name+".tex");
    this->file_mutex.lock();
    QString aa = "(?<=putcomment\\\[.]{).*(?=})";
    process.start("grep", QStringList() << "-oP" << aa);
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
//    QString stderr = process.readAllStandardError();
    this->file_mutex.unlock();
    return stdout;
}

void grader_editor::put_marks(QString file_name, QString marks){
    QProcess process;
    marks=marks.simplified();
    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
    this->file_mutex.lock();
    process.start("cp", QStringList() << file_name+".tex" << "temp1.tex" );
    process.waitForFinished(-1);
    marks=marks.simplified();
    QString temp="s:\\\\putmarks{.*:\\\\putmarks{"+marks+"}:";
    process.setStandardInputFile(this->out_dir_name+"/subfiles/temp1.tex");
    process.setStandardOutputFile(this->out_dir_name+"/subfiles/"+file_name+".tex",QIODevice::Truncate);
    process.start("sed",QStringList() << temp);
    process.waitForFinished(-1);
    process.kill();
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process1.start("rm temp1.tex");
    process1.waitForFinished(-1);
    this->file_mutex.unlock();
}

void grader_editor::put_comment(QString file_name, QString comment){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
    this->file_mutex.lock();
    process.start("cp", QStringList() << file_name+".tex" << "temp2.tex" );
    process.waitForFinished(-1);
//    comment=comment.simplified();
    comment=escape_string(comment);
    QString temp="s:\\\\putcomment\\\[.]{.*:\\\\putcomment\\\["+this->ui->comment_pos_combo->currentText()+"]{"+comment+"}:";
//    qDebug()<<temp;
    process.setStandardInputFile(this->out_dir_name+"/subfiles/temp2.tex");
    process.setStandardOutputFile(this->out_dir_name+"/subfiles/"+file_name+".tex",QIODevice::Truncate);
    process.start("sed",QStringList() << temp);
    process.waitForFinished(-1);
    process.kill();
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process1.start("rm temp2.tex");
    process1.waitForFinished(-1);
    this->file_mutex.unlock();
}

void grader_editor::put_comment_pos(QString file_name, QString comment_pos){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
    this->file_mutex.lock();
    process.start("cp", QStringList() << file_name+".tex" << "temp2.tex" );
    process.waitForFinished(-1);
    QString temp="s:\\\\putcomment\\\[.]{\\(.*\\)}:\\\\putcomment\\\["+comment_pos+"]{\\1}:";
//    qDebug()<<temp;
    process.setStandardInputFile(this->out_dir_name+"/subfiles/temp2.tex");
    process.setStandardOutputFile(this->out_dir_name+"/subfiles/"+file_name+".tex",QIODevice::Truncate);
    process.start("sed",QStringList() << temp);
    process.waitForFinished(-1);
//    qDebug() <<process.readAllStandardError();
    process.kill();
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process1.start("rm temp2.tex");
    process1.waitForFinished(-1);
    this->file_mutex.unlock();
}


void grader_editor::include_only(bool is_include_only){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    process.setStandardInputFile(this->out_dir_name+"/main_pdf.tex");
    this->main_file_mutex.lock();
    process.start("grep",QStringList() << "\\includeonly{.*");
    process.waitForFinished(-1);
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name);
    process1.start("cp main_pdf.tex temp_pdf.tex");
    process1.waitForFinished(-1);
    process1.setStandardInputFile(this->out_dir_name+"/temp_pdf.tex");
    process1.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Truncate);
    QString temp;
    if(process.readAllStandardOutput()==""){
        if(is_include_only){
            temp="1i\\ \\\\includeonly{subfiles\\\/"+this->filesList[this->current_index]+"}";
            process1.start("sed",QStringList()<<temp);
            process1.waitForFinished(-1);
            process1.kill();
        }
    }else{
        if(is_include_only)
            temp="s:\\\\includeonly{.*:\\\\includeonly{subfiles\\\/"+this->filesList[this->current_index]+"}:";
        else
            temp="s:\\\\includeonly{.*::";
        process1.start("sed",QStringList()<<temp);
        process1.waitForFinished(-1);
        process1.kill();

    }
    QProcess process2;
    process2.setWorkingDirectory(this->out_dir_name);
    process2.start("rm temp_pdf.tex");
    process2.waitForFinished(-1);
    this->main_file_mutex.unlock();
}



void grader_editor::on_marks_text_textChanged()
{
    this->future=QtConcurrent::run(this, &grader_editor::preview_thread_func_marks);
}

void grader_editor::on_comment_text_textChanged()
{
    this->future=QtConcurrent::run(this, &grader_editor::preview_thread_func_comment);
}

void grader_editor::on_comment_pos_combo_activated(int index)
{
    this->future=QtConcurrent::run(this, &grader_editor::preview_thread_func_comment_pos,this->ui->comment_pos_combo->currentText());
}

void grader_editor::preview_thread_func_marks(){
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -file-line-error -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    tex_error_p1.setWorkingDirectory(this->out_dir_name);
    tex_error_p2.setWorkingDirectory(this->out_dir_name);
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    this->tex_errors_lock.lockForWrite();
    this->tex_errors=tex_error_p2.readAllStandardOutput();
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
//    process.start("gnome-open", QStringList() << "main_pdf.pdf");
//    process.waitForFinished(-1);
}



void grader_editor::preview_thread_func_comment(){
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -file-line-error -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    tex_error_p1.setWorkingDirectory(this->out_dir_name);
    tex_error_p2.setWorkingDirectory(this->out_dir_name);
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    this->tex_errors_lock.lockForWrite();
    this->tex_errors=tex_error_p2.readAllStandardOutput();
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
//    process.start("gnome-open", QStringList() << "main_pdf.pdf");
//    process.waitForFinished(-1);
}

void grader_editor::preview_thread_func_comment_pos(QString comment_pos){
    put_comment_pos(this->filesList[this->current_index],comment_pos);
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -file-line-error -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process.readAllStandardOutput();
    tex_error_p1.setWorkingDirectory(this->out_dir_name);
    tex_error_p2.setWorkingDirectory(this->out_dir_name);
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    this->tex_errors_lock.lockForWrite();
    this->tex_errors=tex_error_p2.readAllStandardOutput();
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
}


QString grader_editor::escape_string(QString comment){
    comment=comment.simplified();
    comment.replace("\\","\\\\");
    comment.replace("&","\\&");
    comment.replace(":","\\:");
    QString temp="[\\n\\t\\r]";
    qDebug() <<temp;
    comment.replace(temp,"+");
    qDebug() <<comment;
    return comment;
}



void grader_editor::on_fix_file_btn_clicked()
{
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process.start("cp",QStringList()<<this->sub_tex_name<<this->filesList[this->current_index]+".tex");
    process.waitForFinished(-1);
    qDebug() <<process.readAllStandardError();
    qDebug() <<this->sub_tex_name;
    qDebug() <<this->filesList[this->current_index]+".tex";
    //putting put page
    QString file_id=this->filesList[this->current_index].split("_",QString::SkipEmptyParts).last();
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name+"/subfiles");
    process1.start("cp", QStringList() << this->filesList[this->current_index]+".tex" <<"temp.tex");
    process1.waitForFinished(-1);
    process1.setStandardInputFile(this->out_dir_name+"/subfiles/temp.tex");
    process1.setStandardOutputFile(this->out_dir_name+"/subfiles/"+this->filesList[this->current_index]+".tex",QIODevice::Truncate);
    QString temp1="s:\\\\putpage{.*:\\\\putpage{"+file_id+"}:";
    process1.start("sed", QStringList() << temp1 );
    process1.waitForFinished(-1);
    process1.kill();
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(true);
    QProcess process2;
    process2.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process2.start("pdflatex -file-line-error -interaction=nonstopmode main_pdf.tex");
    process2.waitForFinished(-1);
    this->tex_mutex.unlock();
    QProcess tex_error_p1,tex_error_p2;
    QString temp_tex_errors;
    temp_tex_errors=process2.readAllStandardOutput();
    tex_error_p1.setWorkingDirectory(this->out_dir_name);
    tex_error_p2.setWorkingDirectory(this->out_dir_name);
    tex_error_p1.setStandardOutputProcess(&tex_error_p2);
    tex_error_p1.start("echo",QStringList() <<temp_tex_errors);
    QString temp_grep_string=".*:[0-9]*:.*\\|^l.[0-9]*.*";
    tex_error_p1.waitForFinished(-1);
    tex_error_p2.start("grep",QStringList()<<temp_grep_string);
    tex_error_p2.waitForFinished(-1);
    this->tex_errors_lock.lockForWrite();
    this->tex_errors=tex_error_p2.readAllStandardOutput();
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    this->tex_errors_lock.unlock();
}

void grader_editor::on_see_errors_btn_clicked()
{
    QString temp_tex_errors;
    this->tex_errors_lock.lockForRead();
    temp_tex_errors=this->tex_errors;
    this->tex_errors_lock.unlock();
    QMessageBox::warning(
                this,
                tr("Grader"),
                temp_tex_errors );
}
