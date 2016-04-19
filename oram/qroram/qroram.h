#pragma once
// I don't think this file is used anymore TODO delete me
typedef struct QroramTable QroramTable;

QroramTable* qroramTable(int nmax);
void qroramTableRelease(QroramTable* tbl);
int qroramBestPeriod(QroramTable* tbl,int n);
void qroramDebugInfo(QroramTable* tbl);
int qroramShuffTimes(QroramTable* tbl,int** outT,int** outA,int n);
