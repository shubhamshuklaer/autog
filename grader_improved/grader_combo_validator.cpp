#include "grader_combo_validator.h"
#include <QValidator>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QDebug>

grader_combo_validator::grader_combo_validator(QObject * parent,QAbstractItemModel *model):
    QValidator(parent)
{
    this->item_model=model;
}

QValidator::State grader_combo_validator::validate(QString &input, int &pos) const{
    QModelIndex index;
    QString item_text;
    bool acceptable=false,done=false,intermediate=false;
    int i=0;
    int j=this->item_model->rowCount();
    while(i<j&&!done){
        index=this->item_model->index(i,0);
        item_text=this->item_model->data(index).toString();
        if(item_text==input){
            acceptable=true;
            done=true;
        }else{
            item_text.truncate(input.length());
            if(item_text.toLower()==input.toLower())
                intermediate=true;
        }
        i++;
    }
    QValidator::State return_state;
    if(acceptable)
        return_state=QValidator::Acceptable;
    else if(intermediate)
        return_state=QValidator::Intermediate;
    else
        return_state=QValidator::Invalid;
    return return_state;
}

void grader_combo_validator::fixup(QString &input) const{
    QModelIndex index;
    QString item_text;
    bool done=false;
    int i=0;
    int j=this->item_model->rowCount();
    while(i<j&&!done){
        index=this->item_model->index(i,0);
        item_text=this->item_model->data(index).toString();
        item_text.truncate(input.length());
        if(item_text.toLower()==input.toLower()){
            input=this->item_model->data(index).toString();
            done=true;
        }
        i++;
    }
}
