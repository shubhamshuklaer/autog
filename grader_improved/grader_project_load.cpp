#include "grader_project_load.h"
#include "ui_grader_project_load.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCompleter>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include "constants.h"

QString latex_compile_command;

grader_project_load::grader_project_load(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grader_project_load)
{
    ui->setupUi(this);
    QCompleter *module_completer=new QCompleter(this->ui->select_module_combo);
    module_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    module_completer->setCaseSensitivity(Qt::CaseInsensitive);
    module_completer->setModel(this->ui->select_module_combo->model());
    this->ui->select_module_combo->setCompleter(module_completer);
}

grader_project_load::~grader_project_load()
{
    delete ui;
}

void grader_project_load::on_select_project_btn_clicked()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir_name!=NULL){
        QDir dir=QDir(dir_name);
        this->ui->project_dir_name->setText(dir_name);
        if(dir.exists(project_config_name)){
            if(parse_project_config(dir_name)){
                this->ui->select_module_widget->setEnabled(true);
                load_settings();
            }
        }else{
            QMessageBox::warning(
                        this,
                        tr("Grader"),
                        tr("Not a valid autog project, couldn't find ")+project_config_name );
        }
    }
}

void grader_project_load::on_start_btn_clicked()
{
    if(this->ui->select_module_combo->currentIndex()!=-1){
        QString temp_module_name=this->ui->select_module_combo->itemText(this->ui->select_module_combo->currentIndex());
        QDir temp_dir(this->project_path);
        if(temp_dir.exists(temp_module_name)){
            this->module_name=this->ui->select_module_combo->itemText(this->ui->select_module_combo->currentIndex());
            if(setup_module())
                emit done();
        }else{
            QMessageBox::warning(
                        this,
                        tr("Grader"),
                        tr("module directory named ")+temp_module_name+tr(" doens't exist"));
        }
    }else{
        QMessageBox::warning(
                    this,
                    tr("Grader"),
                    tr("Please a module") );
    }

}

bool grader_project_load::parse_project_config(QString project_location){
    QFile project_config_file(project_location+"/"+project_config_name);
    if (!project_config_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(
                    this,
                    tr("Grader"),
                    tr("couldn't find ")+project_config_name );
        return false;
    }
    QTextStream config_text(&project_config_file);
    while(!config_text.atEnd()){
        this->ui->select_module_combo->addItem(config_text.readLine());
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

QStringList grader_project_load::get_filesList(){
    return this->filesList;
}

QStringList grader_project_load::get_marks_denominations(){
    return this->marks_denominations;
}

bool grader_project_load::setup_module(){
    QString module_dir_name=this->project_path+"/"+this->module_name;
    QString sub_tex_path=this->project_path+"/"+const_sub_tex_name;
    QString top_tex_path=this->project_path+"/"+const_top_tex_name;
    QString bursts_dir_path=this->project_path+"/"+const_bursts_dir_name;

    QProcess process;
    process.setWorkingDirectory(module_dir_name);
    process.start("mkdir "+const_out_dir_name);
    process.waitForFinished(-1);
    process.setWorkingDirectory(module_dir_name+"/"+const_out_dir_name);

    QDir project_dir(this->project_path);

    if(!project_dir.exists(const_top_tex_name)){
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("couldn't find preamble file ")+this->project_path+"/"+const_top_tex_name);
        return false;
    }
    if(!project_dir.exists(const_sub_tex_name)){
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("couldn't find sub file ")+this->project_path+"/"+const_sub_tex_name);
        return false;
    }

    if(!project_dir.exists(const_bursts_dir_name)){
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("couldn't find bursts dir ")+this->project_path+"/"+const_bursts_dir_name);
        return false;
    }


    QFile module_config_file(this->project_path+"/"+this->module_name+"/"+module_config_file_name);
    if(!module_config_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open module config file ")+this->project_path+"/"+this->module_name+"/"+module_config_file_name);
        return false;
    }


    QTextStream in(&module_config_file);
    QString temp_line;
    QStringList temp_split;
    while(!in.atEnd()){
        temp_line=in.readLine();
        if(temp_line!=NULL){
            temp_split=temp_line.split(id_marks_delimiter);
            this->filesList<<temp_split[0];
            this->marks_denominations<<temp_split[1];
        }
    }

    module_config_file.close();


    if(this->filesList.length()==0){
        QMessageBox::warning(this,tr("Error"),tr("No id's in the module config file ")+this->project_path+"/"+this->module_name+"/"+module_config_file_name);
        return false;
    }

    for(int i=0;i<this->filesList.length();i++){
        QString temp="bash -c \"if \[ ! -f "+this->filesList[i]+".tex \]; then cp "+sub_tex_path+" "+filesList[i]+".tex; fi;\"";
        process.start(temp);
        process.waitForFinished(-1);
    }


    process.setWorkingDirectory(module_dir_name);
    process.start("cp",QStringList() << top_tex_path << const_main_pdf_name+".tex");
    process.waitForFinished(-1);

    process.setStandardOutputFile(module_dir_name+"/"+const_main_pdf_name+".tex",QIODevice::Append);
    process.start("echo",QStringList() << "\\begin{document}");
    process.waitForFinished(-1);
    process.kill();
    for(int i=0;i<this->filesList.length();i++){
        QProcess process1;
        process1.setWorkingDirectory(module_dir_name);
        process1.setStandardOutputFile(module_dir_name+"/"+const_main_pdf_name+".tex",QIODevice::Append);
        process1.start("echo",QStringList() << "\\include{"+const_out_dir_name+"/"+this->filesList[i]+"}");
        process1.waitForFinished(-1);
        process1.kill();
    }
    QProcess process2;
    process2.setWorkingDirectory(module_dir_name);
    process2.setStandardOutputFile(module_dir_name+"/"+const_main_pdf_name+".tex",QIODevice::Append);
    process2.start("echo",QStringList() << "\\end{document}");
    process2.waitForFinished(-1);
    process2.kill();

    QProcess process3;
    process3.setWorkingDirectory(module_dir_name);
    process3.start("cp",QStringList() << const_main_pdf_name+".tex" << "temp.tex");
    process3.waitForFinished(-1);
    process3.setStandardInputFile(module_dir_name+"/temp.tex");
    process3.setStandardOutputFile(module_dir_name+"/"+const_main_pdf_name+".tex",QIODevice::Truncate);
    QString temp="s:\\\\newcommand{\\\\burstsdir}{.*:\\\\newcommand{\\\\burstsdir}{"+bursts_dir_path+"}:";
    process3.start("sed",QStringList()<<temp);
    process3.waitForFinished(-1);
    process3.kill();

    QProcess process4;
    process4.setWorkingDirectory(module_dir_name);
    process4.start("rm", QStringList() << "temp.tex");
    process4.waitForFinished(-1);


    QString temp1;
    for(int i=0;i<this->filesList.length();i++){
        QProcess process1;
        process1.setWorkingDirectory(module_dir_name+"/"+const_out_dir_name);
        process1.start("cp", QStringList() << this->filesList[i]+".tex" <<"temp.tex");
        process1.waitForFinished(-1);
        process1.setStandardInputFile(module_dir_name+"/"+const_out_dir_name+"/temp.tex");
        process1.setStandardOutputFile(module_dir_name+"/"+const_out_dir_name+"/"+this->filesList[i]+".tex",QIODevice::Truncate);
        temp1="s:\\\\putpage{.*:\\\\putpage{"+this->filesList[i]+"}:";
        process1.start("sed", QStringList() << temp1 );
        process1.waitForFinished(-1);
        process1.kill();
    }

    QProcess process5;
    process5.setWorkingDirectory(module_dir_name+"/"+const_out_dir_name);
    process5.start("rm temp.tex");
    process5.waitForFinished(-1);
    return true;
}


void grader_project_load::load_settings(){
    latex_compile_command=QString();
    QDir project_dir(this->project_path);
    if(project_dir.exists(settings_config_name)){
        QFile settings_config_file(this->project_path+"/"+this->module_name+"/"+settings_config_name);
        if(!settings_config_file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::warning(this,tr("Error"),tr("couldn't open settings config file ")+this->project_path+"/"+this->module_name+"/"+settings_config_name);
            return;
        }

        QTextStream in(&settings_config_file);
        QString temp_line;
        QStringList temp_split;
        while(!in.atEnd()){
            temp_line=in.readLine();
            if(temp_line!=NULL){
                temp_split=temp_line.split(settings_delemiter);
                if(temp_split[0]==latex_compile_command_setting_name&&temp_split[1]!=NULL){
                    latex_compile_command=temp_split[1];
                    qDebug() <<"latex compile set";
                }
            }
        }
    }

    if(latex_compile_command==NULL){
        latex_compile_command=default_latex_compile_command;
        qDebug()<<"Default set";
    }
    qDebug() <<latex_compile_command;
}
