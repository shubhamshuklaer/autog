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
    grader_setup.cpp \
    grader_editor.cpp \
    grader_settings.cpp \
    grader_shortcuts.cpp \
    latex_highlight.cpp \
    grader_marks_widget.cpp

HEADERS  += mainwindow.h \
    grader_setup.h \
    grader_editor.h \
    grader_settings.h \
    grader_shortcuts.h \
    latex_highlight.h \
    grader_marks_widget.h

FORMS    += mainwindow.ui \
    grader_setup.ui \
    grader_editor.ui \
    grader_settings.ui \
    grader_shortcuts.ui \
    grader_marks_widget.ui
