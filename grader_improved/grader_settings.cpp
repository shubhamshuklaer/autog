#include "grader_settings.h"
#include "ui_grader_settings.h"
#include <QMessageBox>

grader_settings::grader_settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::grader_settings)
{
    ui->setupUi(this);
}

grader_settings::~grader_settings()
{
    delete ui;
}

void grader_settings::on_load_default_settings_clicked()
{
    QMessageBox::information(
                this,
                tr("Application Name"),
                tr("Please fill all entries"));
}

void grader_settings::on_save_settings_btn_clicked()
{

}

void grader_settings::on_cancel_settings_btn_clicked()
{

}
