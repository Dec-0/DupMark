#ifndef _CONSENSUSRELATED_H
#define _CONSENSUSRELATED_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "BasicRelated.h"
#include "BamRelated.h"


// 非UMI条件下的标记重复；
unsigned int PairReadsNumConfirm(unsigned int From,unsigned int To,unsigned int *SortInfo,unsigned int *GroupInfo);
unsigned int PairReadsConfirm(unsigned int From,unsigned int To,unsigned int *SortInfo,unsigned int *GroupInfo,unsigned char *Info4Index,unsigned int *PairFrom,unsigned int *PairTo,unsigned int *Read1,unsigned int *Read2);
// 非UMI条件下的去重；
unsigned int DupMarkWithOutUMI(time_t start,unsigned int MapReadsNum,unsigned int *GroupInfo4ReadsID,unsigned int *SortInfo4MapFull,unsigned int *GroupInfo4MapFull,unsigned char *Info4Index,unsigned int *Info4BQ,unsigned int BamGenJumpFlag,unsigned char *DupLogPath);

// UMI条件下的去重；
unsigned int DupMarkWithUMI(time_t start,unsigned char *SamFile,unsigned int MapReadsNum,unsigned long *Info4Shift,unsigned int *GroupInfo4ReadsID,unsigned int *SortInfo4MapFull,unsigned int *GroupInfo4MapFull,unsigned char *Info4Index,unsigned char *DupLogPath,unsigned char *UmiName,unsigned int HUmiSize,unsigned int FakeFlag);
// UMI条件下的重复数量统计；
unsigned int DupCountWithUMI(time_t start,unsigned int MapReadsNum,unsigned int *GroupInfo4ReadsID,unsigned int *SortInfo4MapFull,unsigned int *GroupInfo4MapFull,unsigned char *Info4Index,unsigned long *Info4UMISeq,unsigned char *DupLogPath,unsigned int HUmiSize,unsigned int FakeFlag);

#endif
