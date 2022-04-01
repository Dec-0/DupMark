#ifndef _SORTRELATED_H
#define _SORTRELATED_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "BasicRelated.h"

// 用来对单一数组（Int或者Long类型）进行排序，并确认排序和分类结果；
unsigned int SortAndGroup4Int(unsigned int *Info,unsigned int *Index[],unsigned int Num);
unsigned int SortAndGroup4Long(unsigned long *Info,unsigned int *Index[],unsigned int Num);

// 对Mapping Info进行规范化处理
unsigned int MapInfoStandard(unsigned char *Chr,unsigned int *Pos,unsigned char *PChr,unsigned int *PPos,unsigned int Num,unsigned int Flag);

// 比较坐标大小；
unsigned int MapInfoCompare(unsigned char *Chr,unsigned int *Pos,unsigned char *PChr,unsigned int *PPos,unsigned int LeftBegin,unsigned int RighgBegin);

// 用来对比对数据进行排序，返回排序和分类结果；
unsigned int SortAndGroup4MapInfo(time_t start,unsigned char *Chr,unsigned int *Pos,unsigned char *PChr,unsigned int *PPos,unsigned int *Index[],unsigned int *Group,unsigned int Num,unsigned int DebugFlag);
// 给定一个完整的全序列参考，在限定范围内再排序；
unsigned int ReSortByString(unsigned int From,unsigned int To,unsigned int *SortInfo,unsigned int *GroupInfo);
// 给定的是一个限定范围的参考值
unsigned int UMISeqCompare(unsigned long UMIA,unsigned long UMIB,unsigned int HUmiSize);
unsigned int ReSortByUMI(unsigned int PairNum,unsigned int *PairFrom,unsigned int *PairTo,unsigned int *Read1,unsigned int *Read2,unsigned long *UMISeq,unsigned int HUmiSize,unsigned char *FRFlag);

#endif
