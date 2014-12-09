#include "grader_marks_widget.h"
#include "ui_grader_marks_widget.h"
#include <QWidget>
#include <QCheckBox>
#include <QDebug>

grader_marks_widget::grader_marks_widget(QWidget *parent,QStringList marks_denominations) :
    QWidget(parent),
    ui(new Ui::grader_marks_widget)
{
    ui->setupUi(this);
    this->check_box_list=QList<QCheckBox *>();
    int i=0;
    foreach(QString mark,marks_denominations){
        qDebug() <<"marks "<<mark;
        if(mark.toFloat()!=0){
            qDebug() <<mark.toFloat();
            QCheckBox *temp_box=new QCheckBox();
            temp_box->setText(mark);
            this->ui->horizontal_layout->addWidget(temp_box);
            this->check_box_list<<temp_box;
            connect(temp_box,SIGNAL(stateChanged(int)),this,SLOT(marks_check_box_changed()));
            if(i==0)
                QWidget::setTabOrder(this->ui->marks_text,this->check_box_list[i]);
            else
                QWidget::setTabOrder(this->check_box_list[i-1],this->check_box_list[i]);
        }
    }
    this->ui->marks_text->setValidator(new QDoubleValidator(this));
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocusProxy(this->ui->marks_text);
    connect(this->ui->marks_text,SIGNAL(textChanged(QString)),this,SIGNAL(marks_changed()));
}

grader_marks_widget::~grader_marks_widget()
{
    delete ui;
}

QString grader_marks_widget::get_marks(){
    return this->ui->marks_text->text();
}

void grader_marks_widget::put_marks(QString marks){
    this->ui->marks_text->setText(marks);
}

void grader_marks_widget::marks_check_box_changed(){
    float marks=0;
    foreach(QCheckBox *check_box,this->check_box_list){
        if(check_box->isChecked())
            marks+=check_box->text().toFloat();
    }
    qDebug() <<marks;
    qDebug() <<QString::number(marks);
    this->ui->marks_text->setText(QString::number(marks));
}
