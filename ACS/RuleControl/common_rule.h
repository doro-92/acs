#ifndef COMMON_RULE_H
#define COMMON_RULE_H

#include <QVector>
#include <QString>

typedef struct structLexem
{
    quint16 table; //таблица что это
    quint32 num; //номер в таблице
    quint32 numStr; //номер строки в коде
}structLexem;

typedef struct structNode
{
    quint32 numLexem; //номер лексемы
    quint32 expand; //уровень вложения
}structNode;

typedef struct structParam
{
    quint32 numDev; //номер уст-ва
    QString nameParam; //имя параметра
}structParam;






//готовый набор для конечного автомата
typedef struct structFunction
{
    quint32 function;
    QVector <quint16> typeParams; //0 - const, 1- int, 2 - double, 3 - operator
    QVector <QString> params;
}structFunction;


typedef struct structRuleStateMachine
{
    QString nameRule;
    quint32 numStr;
    QVector <QVector <structFunction> > vecConditionsForRule; //вектор условий проверки правил
    QVector <QVector <structFunction> > vecConditionsRule; //условия правила
}RuleStateMachine;

#endif // COMMON_RULE_H
