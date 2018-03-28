#-------------------------------------------------
#
# Project created by QtCreator 2018-03-26T13:46:35
#
#-------------------------------------------------

QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = ACS
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += main.cpp\
        mainwindow.cpp \
    DBControl/dbcontrol.cpp \
    DBControl/threadqueue.cpp \
    DBControl/threadblobtodb.cpp \
    DataPlot/qcustomplot.cpp \
    DataPlot/qplot.cpp \
    Device/device_plc_f5a.cpp \
    RuleControl/RuleControl.cpp \
    RuleControl/ThParse.cpp \
    RuleControl/Parser.cpp \
    Core/Core.cpp \
    Core/cthTranslate.cpp \
    RuleControl/CodeGenerator.cpp \
    RuleControl/SyntaxAnalyzer.cpp \
    RuleControl/RunTimeTranslator.cpp

HEADERS  += mainwindow.h \
    DataPlot/qplot.h \
    DataPlot/qcustomplot.h \
    DBControl/dbcontrol.h \
    DBControl/common.h \
    DBControl/threadqueue.h \
    DBControl/threadblobtodb.h \
    Device/device_plc_f5a.h \
    RuleControl/RuleControl.h \
    RuleControl/ThParse.h \
    RuleControl/Parser.h \
    Core/Core.h \
    Core/cthTranslate.h \
    EXT/ext.h \
    RuleControl/common_rule.h \
    RuleControl/CodeGenerator.h \
    RuleControl/SyntaxAnalyzer.h \
    RuleControl/RunTimeTranslator.h


FORMS    += mainwindow.ui
