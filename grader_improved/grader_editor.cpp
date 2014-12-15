#include "grader_editor.h"
#include "ui_grader_editor.h"
#include <QProcess>
#include <QDebug>
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
    this->marks_widget=NULL;

    this->file_sys_interface=new grader_file_sys(this,this->main_tex_dir_name,this->out_dir_name);

    setup_marks_widget(0);

    this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->filesList[0],"t"));
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

}

grader_editor::~grader_editor()
{
    delete ui;
}

void grader_editor::on_next_btn_clicked()
{
    this->file_sys_interface->put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    this->file_sys_interface->put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->current_index+2==this->filesList.length())
        this->ui->next_btn->setEnabled(false);
    this->ui->prev_btn->setEnabled(true);
    this->current_index++;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    setup_marks_widget(this->current_index);
    this->ui->comment_pos_combo->setCurrentIndex(0);
    this->previous_comment_pos_index=0;
    this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->filesList[this->current_index],"t"));

}

void grader_editor::on_prev_btn_clicked()
{
    this->file_sys_interface->put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    this->file_sys_interface->put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->current_index<=1)
        this->ui->prev_btn->setEnabled(false);
    this->ui->next_btn->setEnabled(true);
    this->current_index--;
    this->ui->file_name_combo->setCurrentIndex(this->current_index);
    setup_marks_widget(this->current_index);
    this->ui->comment_pos_combo->setCurrentIndex(0);
    this->previous_comment_pos_index=0;
    this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->filesList[this->current_index],"t"));
}

void grader_editor::on_preview_btn_clicked()
{
    this->tex_errors=this->file_sys_interface->generate_pdf(this->filesList[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
    QProcess process;
    process.setWorkingDirectory(this->main_tex_dir_name);
    process.start("gnome-open", QStringList() << const_main_pdf_name+".pdf");
    process.waitForFinished(-1);
}


void grader_editor::on_file_name_combo_activated(int index)
{
    this->file_sys_interface->put_marks(this->filesList[this->current_index],this->marks_widget->property("marks").toString());
    this->file_sys_interface->put_comment(this->filesList[this->current_index],this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
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
    this->ui->comment_text->setText(this->file_sys_interface->get_comment(this->filesList[this->current_index],"t"));

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
    this->tex_errors=this->file_sys_interface->generate_pdf(this->filesList[this->current_index],this->marks_widget->property("marks").toString(),this->ui->comment_text->toPlainText(),this->ui->comment_pos_combo->itemText(this->ui->comment_pos_combo->currentIndex()));
    if(this->tex_errors!=""){
        this->ui->see_errors_btn->setEnabled(true);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{color: red;}");
    }else{
        this->ui->see_errors_btn->setEnabled(false);
        this->ui->see_errors_btn->setStyleSheet("QPushButton{}");
    }
}

void grader_editor::on_see_errors_btn_clicked()
{
    QString temp_tex_errors;
    temp_tex_errors=this->tex_errors;
    QMessageBox::warning(
                this,
                tr("Grader"),
                temp_tex_errors );
}



void grader_editor::setup_marks_widget(int index){
    if(this->marks_widget!=NULL)
        delete this->marks_widget;
    this->marks_widget=new grader_marks_widget(this->ui->marks_widget,this->marks_denominations[index].split(marks_denominations_delemiter));
    this->marks_widget->setProperty("marks",this->file_sys_interface->get_marks(this->filesList[index]));
    this->marks_widget->setFixedSize(this->ui->marks_widget->size());
    connect(this->marks_widget,SIGNAL(marks_changed()),this,SLOT(on_marks_text_textChanged()));
    this->ui->marks_label->setBuddy(this->marks_widget);
    this->marks_widget->show();
}



void grader_editor::on_open_tex_btn_clicked()
{
    QProcess process;
    process.setWorkingDirectory(this->out_dir_name);
    process.start("gnome-open", QStringList()<<this->filesList[this->current_index]+".tex");
    process.waitForFinished(-1);
}
