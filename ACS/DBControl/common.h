#ifndef COMMON_H
#define COMMON_H

//#include "EXT/ext.h"

//флаги ошибок
typedef struct structDBErrors
{
    bool Create;
    bool Open;
    bool CreateTable;
    bool WriteToDB;
    bool ReadFromDB;
    bool CacheCommit;
    bool Commit;
    bool THError;
    bool DeQueueInDataFlow;
    bool EnQueueOutDataFlow;
}structDBErrors;

#endif // COMMON_H
