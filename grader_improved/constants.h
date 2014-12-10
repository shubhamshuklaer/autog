#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

const QString project_config_name="autog.config";
const QString module_config_file_name="module.config";
const QString const_out_dir_name="texfiles";
const QString const_top_tex_name="preamble.tex";
const QString const_sub_tex_name="sub_file.tex";
const QChar id_marks_delimiter=';';
const QChar marks_denominations_delemiter='+';
const QChar settings_delemiter='=';
const QString const_main_pdf_name="main_pdf";
const QString const_bursts_dir_name="bursts";
const QString default_latex_compile_command="pdflatex -file-line-error -interaction=nonstopmode";
const QString settings_config_name="settings.config";
const QString latex_compile_command_setting_name="latex_compile_command";


#endif // CONSTANTS_H
