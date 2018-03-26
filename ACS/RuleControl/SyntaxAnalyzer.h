#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

//#include <QVector>
#include "RuleControl/common_rule.h"

class SyntaxAnalyzer
{
public:
    SyntaxAnalyzer()
    {
        iCnt=0;
    }

    bool VerificationCode();

private:
    QVector <QString> *vecTermWords; //1
    QVector <QString> *vecOperators; //2
    QVector <qreal> *vecConst; //3
    QVector <structParam> *vecParam; //4
    QVector <QString> *vecNames; //5
    QString breaks; //6

    QVector <structLexem> *vecLex; //вектор распарсеных лексем
    QVector <QVector <structNode> > *vecNode; //дерево распарсеного кода

    quint32 iCnt; //счетчик лексем

    //выбор операции
    void SelectOp();
    //правило
    bool Rule();
    //режим
    bool Mode();
    //выход на режим
    bool Cond();
    //условие
    bool cIf();




};

#endif // SYNTAXANALYZER_H
