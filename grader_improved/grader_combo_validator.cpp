/*
 *  Copyright (C) 2014 Shubham Shukla <shubham.shukla@iitg.ernet.in>
 *  This file is part of Auto Grader.
 *
 *  Auto Grader is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Auto Grader is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Auto Grader.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QAbstractListModel>
#include <QDebug>
#include <QModelIndex>
#include <QValidator>

#include "grader_combo_validator.h"


grader_combo_validator::grader_combo_validator(QObject * parent,QAbstractItemModel *model):
    QValidator(parent)
{
    this->data_model=model;
}

QValidator::State grader_combo_validator::validate(QString &input_text, int &pos) const{
    bool acceptable,intermediate;
    int current_item_num,num_items;
    QModelIndex item_index;
    QString item_text;
    QValidator::State return_state;

    acceptable=false;
    intermediate=false;
    current_item_num=0;
    num_items=this->data_model->rowCount();

    while( current_item_num < num_items && !acceptable ){
        item_index=this->data_model->index(current_item_num,0);
        item_text=this->data_model->data(item_index).toString();
        if( item_text == input_text )
            acceptable=true;
        else{
            item_text.truncate(input_text.length());
            if( item_text.toLower() == input_text.toLower() )
                intermediate=true;
        }
        current_item_num++;
    }

    if( acceptable )
        return_state=QValidator::Acceptable;
    else if( intermediate )
        return_state=QValidator::Intermediate;
    else
        return_state=QValidator::Invalid;
    return return_state;
}


void grader_combo_validator::fixup(QString &input_text) const{
    bool done;
    int current_item_num,num_items;
    QModelIndex item_index;
    QString item_text;

    done=false;
    current_item_num=0;
    num_items=this->data_model->rowCount();

    while( current_item_num<num_items && !done ){
        item_index=this->data_model->index(current_item_num,0);
        item_text=this->data_model->data(item_index).toString();
        item_text.truncate(input_text.length());
        if( item_text.toLower() == input_text.toLower() ){
            input_text=this->data_model->data(item_index).toString();
            done=true;
        }
        current_item_num++;
    }
}
