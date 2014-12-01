#include "grader_editor.h"
#include "ui_grader_editor.h"

grader_editor::grader_editor(QWidget *parent,QStringList filesList) :
    QWidget(parent),
    ui(new Ui::grader_editor)
{
    ui->setupUi(this);
    this->ui->file_name_combo->addItems(filesList);
}

grader_editor::~grader_editor()
{
    delete ui;
}
