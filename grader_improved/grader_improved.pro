#-------------------------------------------------
#
# Project created by QtCreator 2014-12-01T14:05:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = autog_binary
target.path = /usr/lib/autog/
INSTALLS += target

data.path = /usr/share/autog/data
data.files += data/COPYING.txt
data.files += data/icon.png
INSTALLS += data

shortcutfiles.path = /usr/share/applications/
shortcutfiles.files = data/autog.desktop
INSTALLS += shortcutfiles

starter_script.path= /usr/bin
starter_script.files= data/autog
INSTALLS +=starter_script

libfiles.path=/usr/lib/autog/
libfiles.files +=data/libQt5*
INSTALLS +=libfiles

platform_plugins.path=/usr/lib/autog/platforms
platform_plugins.files += data/platforms/*
INSTALLS +=platform_plugins

TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    grader_editor.cpp \
    grader_marks_widget.cpp \
    grader_project_load.cpp \
    grader_file_sys.cpp \
    grader_combo_validator.cpp \
    lightlatexhighlighter.cpp \
    blockdata.cpp

HEADERS  += mainwindow.h \
    grader_editor.h \
    grader_marks_widget.h \
    grader_project_load.h \
    constants.h \
    grader_file_sys.h \
    grader_combo_validator.h \
    lightlatexhighlighter.h \
    blockdata.h

FORMS    += mainwindow.ui \
    grader_editor.ui \
    grader_marks_widget.ui \
    grader_project_load.ui

OTHER_FILES +=
