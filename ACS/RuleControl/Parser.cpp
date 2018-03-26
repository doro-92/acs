#include "Parser.h"

Parser::Parser(QVector <QString> &vecTermWords, QVector <QString> &vecOperators,
               QVector <qreal> *vecConst, QVector <structParam> *vecParam, QVector<QString> *vecNames,
               QString &breaks, QChar &termComment, QVector <structLexem> *vecLex, QPlainTextEdit *plainTextEdit)
{
    this->vecTermWords=vecTermWords;
    this->vecOperators=vecOperators;
    this->vecConst=vecConst;
    this->vecParam=vecParam;
    this->vecNames=vecNames;
    this->breaks=breaks;
    this->termComment=termComment;
    this->vecLex=vecLex;
    //this->vecNode=vecNode;
    this->plainTextEdit=plainTextEdit;
}

void Parser::GetLexemes(QPlainTextEdit *plainTextEdit2)
{
    plainTextEdit2->clear();
    for (qint32 i=0;i<vecLex->size();i++)
        plainTextEdit2->appendPlainText("Таблица:"+QString::number(vecLex->operator [](i).table)+","
                +"Номер:"+QString::number(vecLex->operator [](i).num)+","
                +"Номер строки:"+QString::number(vecLex->operator [](i).numStr)+";");
}

void Parser::Parse()
{
    vecLex->clear();
    vecConst->clear();
    vecParam->clear();
    vecNames->clear();

    structParam stParam;
    stParam.numDev=0;
    stParam.nameParam="error";
    vecParam->push_back(stParam);

    vecConst->push_back(0);

    vecNames->push_back("0");

    //plainText должен быть без lineWrapMode!!
    for (qint32 i=0;i<plainTextEdit->document()->lineCount(); i++)
    {
        //получили строку
        QString str=plainTextEdit->toPlainText().split(QChar('\n')).at(i);

        //если есть коммент - отрежем строку
        if (str.indexOf(termComment)!=-1)
            str=str.split(termComment).at(0); //отрежем комменты

        QString res;
        //распарсиваем слово
        for (qint32 j=0;j<str.length();j++)
        {   //символ не пробел, не скобка, не оператор
            if (!Space((QString)str[j]) && WhereBr((QString)str[j])==0 && !ThereOp((QString)str[j]))
                res+=str[j];
            else
            {
                //если длина 0, то может скобка или оператор?
                if (!res.length())
                {   //исключим пробел
                    if (!Space((QString)str[j]))
                        FindAndPut((QString)str[j],i);
                    continue;
                }

                //найдем и положим наш токен
                if (res.length()!=0) { FindAndPut(res,i); res=""; }
                //если скобка - то положим ее
                if (WhereBr((QString)str[j])) { FindAndPut((QString)str[j],i); }
                //если оператор - положим
                if (ThereOp((QString)str[j])) { FindAndPut((QString)str[j],i);  }
            }
        }
    }
}


bool Parser::Space(QString str)
{
    if ((str==" ") || (str==NULL) || (str[0]=='\n') || (str[0]=='\t') || (str[0]=='\r'))
        return true;

    return false;
}

quint32 Parser::WhereBr(QString ch)
{
    for (qint32 i=0;i<breaks.length();i++)
        if (breaks[i]==ch[0]) return i+1;
    return 0;
}

bool Parser::ThereOp(QString str)
{
    if (FindInVec(str,vecOperators)!=0)
        return true;
    return false;
}

quint32 Parser::FindInVec(QString &str, QVector<QString> &vec)
{
    for (qint32 i=0;i<vec.size();i++)
    {
        if (vec[i]==str) return i;
    }
    return 0;
}

quint32 Parser::FindInVecConst(QString &str, QVector<qreal> *vec)
{
    for (qint32 i=0;i<vec->size();i++)
    {
        if (vec->operator [](i)==str.toDouble()) return i;
    }
    return 0;
}

quint32 Parser::FindInVecParam(QString &str, QVector<structParam> *vec)
{
    if (str.indexOf(':')==-1) return 0;

    for (qint32 i=0;i<vec->size();i++)
    {
        // 01:ru.01
        bool ok;
        quint32 numDev=str.split(QChar(':')).at(0).toInt(&ok);
        if (!ok) return 0;

        if (vec->operator [](i).numDev==numDev
                && vec->operator [](i).nameParam==str.split(QChar(':')).at(1))
            return i;
    }
    return 0;
}

bool Parser::TryToParam(QString &str, qint32 &numStr)
{
    //если нет разделителя ':' то это не параметр
    if (str.indexOf(':')==-1) return false;

    QString num=str.split(QChar(':')).at(0);
    QString param=str.split(QChar(':')).at(1);
    //если нет символов - то это бред
    if (num.length()==0 || param.length()==0) return false;

    bool ok;
    quint32 numDev=num.toInt(&ok);
    if (!ok) return false;

    structParam stParam;
    stParam.numDev=numDev;
    stParam.nameParam=param;

    vecParam->push_back(stParam);

    structLexem stLex;
    stLex.table=4;
    stLex.num=vecParam->size()-1;
    stLex.numStr=numStr;
    vecLex->push_back(stLex);

    return true;
}

bool Parser::TryToConst(QString &str, qint32 &numStr)
{
    bool ok;
    qreal Const=str.toDouble(&ok);
    if (!ok) return false;

    vecConst->push_back(Const);

    structLexem stLex;
    stLex.table=3;
    stLex.num=vecConst->size()-1;
    stLex.numStr=numStr;
    vecLex->push_back(stLex);
    return true;
}

void Parser::FindAndPut(QString str, qint32 &numStr)
{
    structLexem stLex;
    quint32 tmp;

    if (str.length()==1)
    {
        tmp=WhereBr((QString)str[0]);
        if (tmp!=0)
        {
            stLex.table=6;
            stLex.num=tmp;
            stLex.numStr=numStr;
            vecLex->push_back(stLex);
            return;
        }
    }

    tmp=FindInVec(str,vecTermWords);
    if (tmp!=0)
    {
        stLex.table=1;
        stLex.num=tmp;
        stLex.numStr=numStr;
        vecLex->push_back(stLex);
        return;
    }

    //проверим сначала на параметр, т.к. 01:ru.01
    tmp=FindInVecParam(str,vecParam);

    if (tmp!=0)
    {
        stLex.table=4;
        stLex.num=tmp;
        stLex.numStr=numStr;
        vecLex->push_back(stLex);
        return;
    }

    //попробуем представить что это параметр
    if (TryToParam(str,numStr)) return;

    tmp=FindInVec(str,vecOperators);

    if (tmp!=0)
    {
        stLex.table=2;
        stLex.num=tmp;
        stLex.numStr=numStr;
        vecLex->push_back(stLex);
        return;
    }



    tmp=FindInVecConst(str,vecConst);

    if (tmp!=0)
    {
        stLex.table=3;
        stLex.num=tmp;
        stLex.numStr=numStr;
        vecLex->push_back(stLex);
        return;
    }


    if (TryToConst(str,numStr)) return;

    //если ни к чему не принадлежит - значит имя какое то
    vecNames->push_back(str);
    stLex.table=5;
    stLex.num=vecNames->size()-1;
    stLex.numStr=numStr;
    vecLex->push_back(stLex);

}
