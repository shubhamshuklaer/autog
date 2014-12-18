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
#include <QRegularExpression>
#include "constants.h"
#include "grader_combo_validator.h"

QString latex_compile_command;

grader_project_load::grader_project_load(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grader_project_load)
{
    ui->setupUi(this);
    QCompleter *module_completer=new QCompleter(this->ui->select_module_combo);
    module_completer->setCompletionMode(QCompleter::PopupCompletion);
    module_completer->setCaseSensitivity(Qt::CaseInsensitive);
    module_completer->setModel(this->ui->select_module_combo->model());
    this->ui->select_module_combo->setCompleter(module_completer);
    this->ui->select_module_combo->setValidator(new grader_combo_validator(this,this->ui->select_module_combo->model()));
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


bool grader_project_load::setup_module(){
    QString module_dir_name=this->project_path+"/"+this->module_name;
    QString sub_tex_path=this->project_path+"/"+const_sub_tex_name;
    QString top_tex_path=this->project_path+"/"+const_top_tex_name;
    QString bursts_dir_path=this->project_path+"/"+const_bursts_dir_name;
    QString out_dir_name=module_dir_name+"/"+const_out_dir_name;

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


    QDir module_dir(module_dir_name);//this exists as checked in start btn slot

    if(!(module_dir.exists(const_out_dir_name)||module_dir.mkdir(const_out_dir_name))){
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("couldn't create output dir ")+out_dir_name);
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


    QDir out_dir(out_dir_name);

    for(int i=0;i<this->filesList.length();i++){
        if(!out_dir.exists(out_dir_name+"/"+this->filesList[i]+".tex")){
            if(!QFile::copy(sub_tex_path,out_dir_name+"/"+this->filesList[i]+".tex")){
                //couldn't copy
                QMessageBox::warning(
                            this,
                            tr("Error"),
                            tr("couldn't copy file from ")+sub_tex_path+tr(" to ")+out_dir_name+"/"+this->filesList[i]+".tex");
                return false;
            }
        }
    }

    if(module_dir.exists(const_main_pdf_name+".tex")){
        if(!module_dir.remove(const_main_pdf_name+".tex")){
            QMessageBox::warning(
                        this,
                        tr("Error"),
                        tr("couldn't file ")+module_dir_name+"/"+const_main_pdf_name+".tex" +tr(" exists and couldn't be removed for overwriting"));
            return false;
        }
    }

    if(!QFile::copy(top_tex_path,module_dir_name+"/"+const_main_pdf_name+".tex")){
        QMessageBox::warning(
                    this,
                    tr("Error"),
                    tr("couldn't copy file from ")+top_tex_path+tr(" to ")+module_dir_name+"/"+const_main_pdf_name+".tex");
        return false;
    }

    QStringList put_into_main_pdf;
    put_into_main_pdf<<"\\begin{document}";
    for(int i=0;i<this->filesList.length();i++){
        put_into_main_pdf<<"\\include{"+const_out_dir_name+"/"+this->filesList[i]+"}";
    }
    put_into_main_pdf<< "\\end{document}";

    QFile main_pdf_tex_file(module_dir_name+"/"+const_main_pdf_name+".tex");
    if(!main_pdf_tex_file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+module_dir_name+"/"+const_main_pdf_name+".tex"+tr(" for read"));
        return false;
    }


    QTextStream main_pdf_tex_input_stream(&main_pdf_tex_file);
    QString main_pdf_content=main_pdf_tex_input_stream.readAll();
    main_pdf_tex_file.close();


    if(!main_pdf_tex_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
        QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+module_dir_name+"/"+const_main_pdf_name+".tex"+tr(" for read"));
        return false;
    }


    QTextStream main_pdf_tex_output_stream(&main_pdf_tex_file);
    QRegularExpression burst_path_pattern("\\\\newcommand{\\\\burstsdir}{.*");
    main_pdf_content.replace(burst_path_pattern,"\\newcommand{\\burstsdir}{"+bursts_dir_path+"}");

    main_pdf_tex_output_stream<<main_pdf_content<<"\n";

    //const reference makes it faster as we are not creating any new object
    foreach(const QString &put_temp,put_into_main_pdf)
        main_pdf_tex_output_stream<<put_temp<<"\n";

    main_pdf_tex_file.flush();
    main_pdf_tex_file.close();

    for(int i=0;i<this->filesList.length();i++){
        QFile sub_tex_file(out_dir_name+"/"+this->filesList[i]+".tex");

        if(!sub_tex_file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+out_dir_name+"/"+this->filesList[i]+".tex"+tr(" for read"));
            return false;
        }

        QTextStream sub_tex_input_stream(&sub_tex_file);
        QString sub_tex_content=sub_tex_input_stream.readAll();
        sub_tex_file.close();

        if(!sub_tex_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){
            QMessageBox::warning(this,tr("Error"),tr("couldn't open file ")+out_dir_name+"/"+this->filesList[i]+".tex"+tr(" for read"));
            return false;
        }


        QTextStream sub_tex_output_stream(&sub_tex_file);
        QRegularExpression put_page_pattern("\\\\putpage{.*");
        qDebug()<<"Sub tex content"<<sub_tex_content;
        sub_tex_content.replace(put_page_pattern,"\\putpage{"+this->filesList[i]+"}");
        qDebug()<<sub_tex_content;
        sub_tex_output_stream<<sub_tex_content;

        sub_tex_file.flush();
        sub_tex_file.close();
    }
    return true;
}




















