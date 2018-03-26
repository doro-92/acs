#include "RuleControl.h"


RuleControl::RuleControl(QPlainTextEdit *plainTextEdit)
{
    vecConst=new QVector <qreal>;
    vecParam=new QVector <structParam>;
    vecLex=new QVector <structLexem>;
    vecNode=new QVector <QVector <structNode> >;
    vecNames=new QVector <QString>;

    vecTermWords.push_back("error"); //заглушка
    vecTermWords.push_back("rule"); //правило
    vecTermWords.push_back("mode"); //режим
    vecTermWords.push_back("cond"); //выход на режим
    vecTermWords.push_back("if");
    vecTermWords.push_back("GetSred");
    vecTermWords.push_back("postGetSred");

    vecOperators.push_back("error");//заглушка
    vecOperators.push_back(">");
    vecOperators.push_back("<");
    vecOperators.push_back("=");
    vecOperators.push_back("&"); //И
    vecOperators.push_back("|"); //или
    vecOperators.push_back(";"); //окончание строки или функции
    vecOperators.push_back(","); //для разделения параметров функции (mode(1,2))

    breaks="{}()";

    termComment='/';

    structParam stParam;
    stParam.numDev=0;
    stParam.nameParam="error";
    vecParam->push_back(stParam);

    vecConst->push_back(0);

    vecNames->push_back("0");

    objParser = new Parser(vecTermWords,vecOperators,vecConst,vecParam,vecNames,
                           breaks,termComment,vecLex,plainTextEdit);

}

void RuleControl::Parse(QPlainTextEdit *plainTextEdit)
{
    objParser->Parse();
    objParser->GetLexemes(plainTextEdit);
}
