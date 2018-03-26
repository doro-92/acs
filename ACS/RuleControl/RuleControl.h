#ifndef RULECONTROL_H
#define RULECONTROL_H

#include <QObject>
#include "RuleControl/common_rule.h"
#include "RuleControl/Parser.h"

class RuleControl : public QObject
{
    Q_OBJECT
public:
    RuleControl(QPlainTextEdit *plainTextEdit);

    void Parse(QPlainTextEdit *plainTextEdit);


signals:

public slots:

private:
    //поддержка языка
    QVector <QString> vecTermWords; //терминальные слова, таблица 1
    QVector <QString> vecOperators; //операторы, таблица 2
    QVector <qreal> *vecConst; //вектор распарсеных констант типа double (arm-float), таблица 3
    QVector <structParam> *vecParam; //вектор распарсеных параметров, таблица 4
    QVector <QString> *vecNames; //вектор распарсеных имен, таблица 5
    QString breaks; //поддерживаемые скобки, таблица 6
    QChar termComment;

    QVector <structLexem> *vecLex; //вектор распарсеных лексем
    QVector <QVector <structNode> > *vecNode; //дерево распарсеного кода


    Parser *objParser;

};

#endif // RULECONTROL_H










