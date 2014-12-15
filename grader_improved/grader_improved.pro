#-------------------------------------------------
#
# Project created by QtCreator 2014-12-01T14:05:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = grader_improved
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    grader_editor.cpp \
    grader_marks_widget.cpp \
    grader_project_load.cpp \
    grader_file_sys.cpp

HEADERS  += mainwindow.h \
    grader_setup.h \
    grader_editor.h \
    grader_marks_widget.h \
    grader_project_load.h \
    constants.h \
    grader_file_sys.h

FORMS    += mainwindow.ui \
    grader_setup.ui \
    grader_editor.ui \
    grader_marks_widget.ui \
    grader_project_load.ui
