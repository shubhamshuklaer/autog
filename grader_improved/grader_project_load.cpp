#include "grader_project_load.h"
#include "ui_grader_load.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCompleter>

const QString project_config_name="autog.config";

grader_project_load::grader_project_load(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grader_load)
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
                qDebug()<<"Parse success";
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
    this->ui->select_module_widget->setEnabled(true);
    this->project_path=project_location;
    return true;
}

QString grader_project_load::get_module_name(){
    return this->module_name;
}

QString grader_project_load::get_project_path(){
    return this->project_path;
}
