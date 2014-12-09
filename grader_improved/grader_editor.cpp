#include "grader_editor.h"
#include "ui_grader_editor.h"
#include <QProcess>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QCompleter>
#include "grader_marks_widget.h"
#include <QFile>
#include <QTextStream>
#include "constants.h"



grader_editor::grader_editor(QWidget *parent,QString project_path,QString module_name,QStringList filesList,QStringList marks_denominations) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);
    this->project_path=project_path;
    this->module_name=module_name;
    this->filesList=filesList;
    this->marks_denominations=marks_denominations;

    this->out_dir_name=project_path+"/"+module_name+"/"+const_out_dir_name;
    this->main_tex_dir_name=project_path+"/"+module_name;
    this->current_index=0;
    this->sub_tex_name=project_path+"/"+const_sub_tex_name;
    this->ui->file_name_combo->addItems(this->filesList);
    this->ui->file_name_combo->setCurrentIndex(0);

    setup_marks_widget(0);

    this->ui->comment_text->setText(get_comment(this->filesList[0],"t"));
    if(this->current_index+1>=this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(false);

    QCompleter *completer;
    completer=new QCompleter(this->ui->file_name_combo);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->ui->file_name_combo->model());
    this->ui->file_name_combo->setCompleter(completer);


    completer=new QCompleter(this->ui->comment_pos_combo);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModel(this->ui->comment_pos_combo->model());
    this->ui->comment_pos_combo->setCompleter(completer);

    this->previous_comment_pos_index=0;
}

grader_editor::~grader_editor()
{
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->current_index+2==this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(true);
    this->current_index++;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    setup_marks_widget(this->current_index);
    this->ui->comment_pos_combo->setCurrentIndex(0);
    this->previous_comment_pos_index=0;
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index],"t"));

}

void grader_editor::on_prev_btn_clicked()
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->current_index<=1)
        this->ui->prev_btn->setEnabled(false);
    this->ui->next_btn->setEnabled(true);
    this->current_index--;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    setup_marks_widget(this->current_index);
    this->ui->comment_pos_combo->setCurrentIndex(0);
    this->previous_comment_pos_index=0;
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index],"t"));
}

void grader_editor::on_preview_btn_clicked()
{
    generate_pdf(true);
    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    process.start("gnome-open", QStringList() << const_main_pdf_name+".pdf");
    process.waitForFinished(-1);
}

void grader_editor::on_gen_pdf_btn_clicked()
{
    generate_pdf(false);
    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    process.start("gnome-open", QStringList() << const_main_pdf_name+".pdf");
    process.waitForFinished(-1);
}

void grader_editor::on_file_name_combo_activated(int index)
{
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    this->current_index=index;
    if(this->current_index==0)
        this->ui->prev_btn->setEnabled(false);
    else
        this->ui->prev_btn->setEnabled(true);
    if(this->current_index+1>=this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    else
        this->ui->next_btn->setEnabled(true);
    setup_marks_widget(this->current_index);
    this->ui->comment_pos_combo->setCurrentIndex(0);
    this->previous_comment_pos_index=0;
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index],"t"));

}



QString grader_editor::get_marks(QString file_name){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    process.setStandardInputFile(this->out_dir_name+"/"+file_name+".tex");
    this->file_mutex.lock();
    process.start("grep", QStringList() << "-oP" << "(?<=putmarks{).*(?=})");
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
//    QString stderr = process.readAllStandardError();
    this->file_mutex.unlock();
    return stdout;
}

void grader_editor::put_marks(QString file_name, QString marks){
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


QString grader_editor::get_comment(QString file_name,QString comment_pos){
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

void grader_editor::put_comment(QString file_name, QString comment,QString comment_pos){
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    this->file_mutex.lock();
    process.start("cp", QStringList() << file_name+".tex" << "temp2.tex" );
    process.waitForFinished(-1);
//    comment=comment.simplified();
    comment=escape_string(comment);
    QString temp="s:\\\\putcomment\\\["+comment_pos+"]{.*:\\\\putcomment\\\["+comment_pos+"]{"+comment+"}:";
//    qDebug()<<temp;
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


void grader_editor::include_only(bool is_include_only){
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
            temp="1i\\ \\\\includeonly{"+const_out_dir_name+"\\\/"+this->filesList[this->current_index]+"}";
            process1.start("sed",QStringList()<<temp);
            process1.waitForFinished(-1);
            process1.kill();
        }
    }else{
        if(is_include_only)
            temp="s:\\\\includeonly{.*:\\\\includeonly{"+const_out_dir_name+"\\\/"+this->filesList[this->current_index]+"}:";
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



void grader_editor::on_marks_text_textChanged()
{
    qDebug()<<"marks text changed";
    this->future=QtConcurrent::run(this, &grader_editor::generate_pdf,true);
}

void grader_editor::on_comment_text_textChanged()
{
    this->future=QtConcurrent::run(this, &grader_editor::generate_pdf,true);
}

void grader_editor::on_comment_pos_combo_activated(int index)
{
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->previous_comment_pos_index));
    this->ui->comment_text->setText(get_comment(this->filesList[this->current_index],this->ui->comment_pos_combo->itemText(index)));
    this->previous_comment_pos_index=index;
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
    process.setWorkingDirectory(this->out_dir_name);
    process.start("cp",QStringList()<<this->sub_tex_name<<this->filesList[this->current_index]+".tex");
    process.waitForFinished(-1);
    qDebug() <<process.readAllStandardError();
    //putting put page
    QString file_id=this->filesList[this->current_index];
    QProcess process1;
    process1.setWorkingDirectory(this->out_dir_name);
    process1.start("cp", QStringList() << this->filesList[this->current_index]+".tex" <<"temp.tex");
    process1.waitForFinished(-1);
    process1.setStandardInputFile(this->out_dir_name+"/temp.tex");
    process1.setStandardOutputFile(this->out_dir_name+"/"+this->filesList[this->current_index]+".tex",QIODevice::Truncate);
    QString temp1="s:\\\\putpage{.*:\\\\putpage{"+file_id+"}:";
    process1.start("sed", QStringList() << temp1 );
    process1.waitForFinished(-1);
    process1.kill();
    generate_pdf(true);
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

void grader_editor::generate_pdf(bool is_include_only){
    put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    include_only(is_include_only);
    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    this->tex_mutex.lock();
    process.start("pdflatex -file-line-error -interaction=nonstopmode "+const_main_pdf_name+".tex");
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

void grader_editor::setup_marks_widget(int index){
    delete this->marks_widget;
    this->marks_widget=new grader_marks_widget(this->ui->marks_widget,this->marks_denominations[index].split(marks_denominations_delemiter));
    this->marks_widget->setProperty("marks",get_marks(this->filesList[index]));
    connect(this->marks_widget,SIGNAL(marks_changed()),this,SLOT(on_marks_text_textChanged()));
    this->ui->marks_label->setBuddy(this->marks_widget);
    this->marks_widget->show();
}
