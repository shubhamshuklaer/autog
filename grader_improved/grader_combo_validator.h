#ifndef GRADER_COMBO_VALIDATOR_H
#define GRADER_COMBO_VALIDATOR_H

#include <QAbstractItemModel>
#include <QValidator>

class grader_combo_validator :public QValidator
{
    Q_OBJECT;
public:
    explicit grader_combo_validator(QObject * parent=0,QAbstractItemModel *model=NULL);
    QValidator::State validate(QString &, int &) const;
    void fixup(QString &input) const;
private:
    QAbstractItemModel *data_model;
};

#endif // GRADER_COMBO_VALIDATOR_H
