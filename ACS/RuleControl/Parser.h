#ifndef PARSER_H
#define PARSER_H

#include <QVector>
#include <QtWidgets/QPlainTextEdit>
#include "RuleControl/common_rule.h"

class Parser
{
public:
    Parser(QVector <QString> &vecTermWords, QVector <QString> &vecOperators,
           QVector <qreal> *vecConst, QVector <structParam> *vecParam,
           QVector <QString> *vecNames, QString &breaks,
           QChar &termComment, QVector <structLexem> *vecLex,
           QPlainTextEdit *plainTextEdit);

    //bool Parse();
    QVector <QVector <structNode> > GetNodes();
    void GetLexemes(QPlainTextEdit *plainTextEdit2);

    //запустить парсинг
    void Parse();

private:
    QPlainTextEdit *plainTextEdit;

    QVector <QString> vecTermWords; //1
    QVector <QString> vecOperators; //2
    QVector <qreal> *vecConst; //3
    QVector <structParam> *vecParam; //4
    QVector <QString> *vecNames; //5
    QString breaks; //6
    QChar termComment;
    QVector <structLexem> *vecLex;
    //QVector <QVector <structNode> > *vecNode;


    //-------helpers functions----------//

    //является ли символ пробелом или новой строкой
    bool Space(QString str);
    //проверка на скобки
    quint32 WhereBr(QString ch);
    //проверка на оператор
    bool ThereOp(QString str);
    //поиск в векторе терменального слова итд
    quint32 FindInVec(QString &str,QVector <QString> &vec);
    //поиск в векторе констант
    quint32 FindInVecConst(QString &str,QVector <qreal> *vec);
    //поиск в векторе параметров
    quint32 FindInVecParam(QString &str,QVector <structParam> *vec);

    //пытается сделать параметр и записать
    bool TryToParam(QString &str, qint32 &numStr);

    //пытается сделать константу, и записать ее
    bool TryToConst(QString &str, qint32 &numStr);
    //поиск выпарсеного фрагмента во всех таблицах и добавление его
    void FindAndPut(QString str, qint32 &numStr);



};

#endif // PARSER_H
