#include "grader_editor.h"
#include "ui_grader_editor.h"
#include <QProcess>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

grader_editor::grader_editor(QWidget *parent,QStringList filesList,QString out_dir_name) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);
    this->filesList=filesList;
    this->out_dir_name=out_dir_name;
    this->current_index=0;
    this->ui->file_name_combo->addItems(this->filesList);
    this->ui->file_name_combo->setCurrentIndex(0);
    this->ui->marks_text->setText(get_marks(this->filesList[0]));
    this->ui->comment_text->setText(get_comment(this->filesList[0]));
    if(this->current_index+1>=this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(false);
}

grader_editor::~grader_editor()
{
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->ui->marks_text->text());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    if(this->current_index+2==this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(true);
    this->current_index++;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    this->ui->marks_text->setText(get_marks(this->filesList[this->current_index]));
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index]));

}

void grader_editor::on_prev_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->ui->marks_text->text());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    if(this->current_index<=1)
        this->ui->prev_btn->setEnabled(false);
    this->ui->next_btn->setEnabled(true);
    this->current_index--;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    this->ui->marks_text->setText(get_marks(this->filesList[this->current_index]));
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index]));
}

void grader_editor::on_preview_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->ui->marks_text->text());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -synctex=1 -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    process.start("gnome-open", QStringList() << "main_pdf.pdf");
    process.waitForFinished(-1);
}

void grader_editor::on_gen_pdf_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->ui->marks_text->text());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(false);
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -synctex=1 -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
    process.start("gnome-open", QStringList() << "main_pdf.pdf");
    process.waitForFinished(-1);
}

void grader_editor::on_file_name_combo_activated(int index)
{
    put_marks(this->filesList[this->current_index],this->ui->marks_text->text());
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
    this->ui->marks_text->setText(get_marks(this->filesList[this->current_index]));
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
    comment=comment.simplified();
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
    process.start("grep",QStringList() << "\includeonly{*");
    process.waitForFinished(-1);
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name);
    process1.start("cp main_pdf.tex temp_pdf.tex");
    process1.waitForFinished(-1);
    process1.setStandardInputFile(this->out_dir_name+"/temp_pdf.tex");
    process1.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex");
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
    process.setWorkingDirectory(this->out_dir_name);
    process2.start("rm temp_pdf.tex");
    process2.waitForFinished(-1);
    this->main_file_mutex.unlock();
}



void grader_editor::on_marks_text_textChanged(const QString &arg1)
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
    put_marks(this->filesList[this->current_index],this->ui->marks_text->text());
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -synctex=1 -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
//    process.start("gnome-open", QStringList() << "main_pdf.pdf");
//    process.waitForFinished(-1);
}



void grader_editor::preview_thread_func_comment(){
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText());
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -synctex=1 -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
//    process.start("gnome-open", QStringList() << "main_pdf.pdf");
//    process.waitForFinished(-1);
}

void grader_editor::preview_thread_func_comment_pos(QString comment_pos){
    put_comment_pos(this->filesList[this->current_index],comment_pos);
    include_only(true);
    QProcess process;
    process.setWorkingDirectory(out_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -synctex=1 -interaction=nonstopmode main_pdf.tex");
    process.waitForFinished(-1);
    this->tex_mutex.unlock();
}


