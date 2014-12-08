//#include <QFileDialog>
//#include <QString>
//#include <QProcess>
//#include <QDebug>
//#include <QMessageBox>
//#include "grader_setup.h"
//#include "ui_grader_setup.h"

//grader_setup::grader_setup(QWidget *parent) :
//    QWidget(parent),
//    ui(new Ui::grader_setup)
//{
//    ui->setupUi(this);
//    this->burst_dir_name=QString();
//    this->filesList=QStringList();
//    this->burst_dir=QDir();
//    connect( this->ui->choose_folder_btn, SIGNAL( clicked() ), this, SLOT(choose_folder()) );
//    connect( this->ui->choose_out_folder_btn, SIGNAL( clicked() ), this, SLOT(choose_out_folder()) );
//    connect( this->ui->choose_top_btn, SIGNAL( clicked() ), this, SLOT(choose_top()) );
//    connect( this->ui->choose_subfile_btn, SIGNAL( clicked() ), this, SLOT(choose_subfile()) );
//    connect( this->ui->setup_output_btn, SIGNAL(clicked()),this,SLOT(setup_output()));
//}

//grader_setup::~grader_setup()
//{
//    delete ui;
//}


//void grader_setup::choose_folder(){
//    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
//                                                 "/home",
//                                                 QFileDialog::ShowDirsOnly
//                                                 | QFileDialog::DontResolveSymlinks);

//    if(dir!=NULL){
//        this->burst_dir_name=dir;
//        this->ui->burst_folder_name->setText(dir);
//        this->burst_dir=QDir(dir);
//    }
//}

//void grader_setup::choose_top(){
//    QString fileName = QFileDialog::getOpenFileName(this,
//       tr("Open tex file"), "/home", tr("tex Files (*.tex)"));
//    if(fileName!=NULL){
//        this->top_tex_name=fileName;
//        this->ui->top_tex_name->setText(fileName);
//    }
//}

//void grader_setup::choose_subfile(){
//    QString fileName = QFileDialog::getOpenFileName(this,
//       tr("Open tex file"),"/home", tr("tex Files (*.tex)"));
//    if(fileName!=NULL){
//        this->sub_tex_name=fileName;
//        this->ui->sub_tex_name->setText(fileName);
//    }
//}


//void grader_setup::choose_out_folder(){
//    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
//                                                 "/home",
//                                                 QFileDialog::ShowDirsOnly
//                                                 | QFileDialog::DontResolveSymlinks);

//    if(dir!=NULL){
//        this->out_dir_name=dir;
//        this->ui->out_folder_name->setText(dir);
//    }
//}

//void grader_setup::setup_output(){
//    QProcess process;
//    QFileInfoList files_info_list;
//    process.setWorkingDirectory(this->out_dir_name);
//    process.start("mkdir subfiles");
//    process.waitForFinished(-1);
//    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
//    if(this->burst_dir_name!=NULL&&this->top_tex_name!=NULL&&this->sub_tex_name!=NULL){
//        files_info_list=this->burst_dir.entryInfoList(QDir::Files,QDir::Name);
//        for(int i=0;i<files_info_list.length();i++){
//            if(files_info_list[i].completeSuffix()=="pdf"){
//                this->filesList.push_back(files_info_list[i].baseName());
//            }
//        }
//        for(int i=0;i<this->filesList.length();i++){
//            QString temp="bash -c \"if \[ ! -f "+this->filesList[i]+".tex \]; then cp "+this->sub_tex_name+" "+filesList[i]+".tex; fi;\"";
////            qDebug() <<temp;
//            process.start(temp);
//            process.waitForFinished(-1);
////            qDebug() <<process.readAllStandardError();
//        }
//        process.setWorkingDirectory(this->out_dir_name);
//        process.start("cp",QStringList() << this->top_tex_name << "main_pdf.tex");
//        process.waitForFinished(-1);
//        process.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Append);
//        process.start("echo",QStringList() << "\\begin{document}");
//        process.waitForFinished(-1);
//        process.kill();
//        for(int i=0;i<this->filesList.length();i++){
//            QProcess process1;
//            process1.setWorkingDirectory(this->out_dir_name);
//            process1.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Append);
//            process1.start("echo",QStringList() << "\\include{subfiles/"+this->filesList[i]+"}");
//            process1.waitForFinished(-1);
//            process1.kill();
//        }
//        QProcess process2;
//        process2.setWorkingDirectory(this->out_dir_name);
//        process2.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Append);
//        process2.start("echo",QStringList() << "\\end{document}");
//        process2.waitForFinished(-1);
//        process2.kill();

//        QProcess process3;
//        process3.setWorkingDirectory(this->out_dir_name);
//        process3.start("cp",QStringList() << "main_pdf.tex" << "temp.tex");
//        process3.waitForFinished(-1);
//        process3.setStandardInputFile(this->out_dir_name+"/temp.tex");
//        process3.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Truncate);
//        QString temp="s:\\\\newcommand{\\\\burstsdir}{.*:\\\\newcommand{\\\\burstsdir}{"+this->burst_dir_name+"}:";
//        process3.start("sed",QStringList()<<temp);
//        process3.waitForFinished(-1);
//        process3.kill();

//        QProcess process4;
//        process4.setWorkingDirectory(this->out_dir_name);
//        process4.start("rm", QStringList() << "temp.tex");
//        process4.waitForFinished(-1);

//        QString temp1;
//        for(int i=0;i<this->filesList.length();i++){
//            QString file_id=this->filesList[i].split("_",QString::SkipEmptyParts).last();
//            QProcess process1;
//            process1.setWorkingDirectory(this->out_dir_name+"/subfiles");
//            process1.start("cp", QStringList() << this->filesList[i]+".tex" <<"temp.tex");
//            process1.waitForFinished(-1);
//            process1.setStandardInputFile(this->out_dir_name+"/subfiles/temp.tex");
//            process1.setStandardOutputFile(this->out_dir_name+"/subfiles/"+this->filesList[i]+".tex",QIODevice::Truncate);
//            temp1="s:\\\\putpage{.*:\\\\putpage{"+file_id+"}:";
//            process1.start("sed", QStringList() << temp1 );
//            process1.waitForFinished(-1);
//            process1.kill();
//        }

//        QProcess process5;
//        process5.setWorkingDirectory(this->out_dir_name+"/subfiles");
//        process5.start("rm temp.tex");
//        process5.waitForFinished(-1);
//        emit done();
//    }else{
//        QMessageBox::information(
//            this,
//            tr("Application Name"),
//            tr("Please fill all entries") );
//    }

//}



//void grader_setup::on_start_new_btn_clicked()
//{
//    QProcess process;
//    QFileInfoList files_info_list;
//    process.setWorkingDirectory(this->out_dir_name);
//    process.start("mkdir subfiles");
//    process.waitForFinished(-1);
//    process.setWorkingDirectory(this->out_dir_name+"/subfiles");
//    if(this->burst_dir_name!=NULL&&this->top_tex_name!=NULL&&this->sub_tex_name!=NULL){
//        files_info_list=this->burst_dir.entryInfoList(QDir::Files,QDir::Name);
//        for(int i=0;i<files_info_list.length();i++){
//            if(files_info_list[i].completeSuffix()=="pdf"){
//                this->filesList.push_back(files_info_list[i].baseName());
//            }
//        }
//        for(int i=0;i<this->filesList.length();i++){
//            QString temp="cp "+this->sub_tex_name+" "+filesList[i]+".tex";
////            qDebug() <<temp;
//            process.start(temp);
//            process.waitForFinished(-1);
////            qDebug() <<process.readAllStandardError();
//        }
//        process.setWorkingDirectory(this->out_dir_name);
//        process.start("cp",QStringList() << this->top_tex_name << "main_pdf.tex");
//        process.waitForFinished(-1);
//        process.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Append);
//        process.start("echo",QStringList() << "\\begin{document}");
//        process.waitForFinished(-1);
//        process.kill();
//        for(int i=0;i<this->filesList.length();i++){
//            QProcess process1;
//            process1.setWorkingDirectory(this->out_dir_name);
//            process1.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Append);
//            process1.start("echo",QStringList() << "\\include{subfiles/"+this->filesList[i]+"}");
//            process1.waitForFinished(-1);
//            process1.kill();
//        }
//        QProcess process2;
//        process2.setWorkingDirectory(this->out_dir_name);
//        process2.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Append);
//        process2.start("echo",QStringList() << "\\end{document}");
//        process2.waitForFinished(-1);
//        process2.kill();

//        QProcess process3;
//        process3.setWorkingDirectory(this->out_dir_name);
//        process3.start("cp",QStringList() << "main_pdf.tex" << "temp.tex");
//        process3.waitForFinished(-1);
//        process3.setStandardInputFile(this->out_dir_name+"/temp.tex");
//        process3.setStandardOutputFile(this->out_dir_name+"/main_pdf.tex",QIODevice::Truncate);
//        QString temp="s:\\\\newcommand{\\\\burstsdir}{.*:\\\\newcommand{\\\\burstsdir}{"+this->burst_dir_name+"}:";
//        process3.start("sed",QStringList()<<temp);
//        process3.waitForFinished(-1);
//        process3.kill();

//        QProcess process4;
//        process4.setWorkingDirectory(this->out_dir_name);
//        process4.start("rm", QStringList() << "temp.tex");
//        process4.waitForFinished(-1);

//        QString temp1;
//        for(int i=0;i<this->filesList.length();i++){
//            QString file_id=this->filesList[i].split("_",QString::SkipEmptyParts).last();
//            QProcess process1;
//            process1.setWorkingDirectory(this->out_dir_name+"/subfiles");
//            process1.start("cp", QStringList() << this->filesList[i]+".tex" <<"temp.tex");
//            process1.waitForFinished(-1);
//            process1.setStandardInputFile(this->out_dir_name+"/subfiles/temp.tex");
//            process1.setStandardOutputFile(this->out_dir_name+"/subfiles/"+this->filesList[i]+".tex",QIODevice::Truncate);
//            temp1="s:\\\\putpage{.*:\\\\putpage{"+file_id+"}:";
//            process1.start("sed", QStringList() << temp1 );
//            process1.waitForFinished(-1);
//            process1.kill();
//        }

//        QProcess process5;
//        process5.setWorkingDirectory(this->out_dir_name+"/subfiles");
//        process5.start("rm temp.tex");
//        process5.waitForFinished(-1);
//        emit done();
//    }else{
//        QMessageBox::information(
//            this,
//            tr("Application Name"),
//            tr("Please fill all entries") );
//    }
//}
