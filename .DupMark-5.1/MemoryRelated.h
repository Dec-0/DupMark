#ifndef _MEMORYRELATED_H
#define _MEMORYRELATED_H

#include <stdio.h>
#include <stdlib.h>

// 分配Bam行数 * 8 Bytes的空间，用于存储ReadsID信息;
unsigned int MemoryRequireOfInfo4IDFull(unsigned int ArraySize,unsigned long *GeneralPS4Long[]);
unsigned int MemoryFreeOfInfo4IDFull(unsigned long *Info4IDFull);
unsigned int MemoryFreeOfSortInfo4ReadsID(unsigned int *SortInfo4ReadsID);

// 一个二维数组，专门用于排序；
unsigned int MemoryRequireOfSort(unsigned int ArraySize,unsigned int *Index4Int[]);
unsigned int MemoryFreeOfSortBase(unsigned int *Index[]);
unsigned int MemoryFreeOfSortFull(unsigned int *Index[]);
unsigned int MemoryFreeOfSortOne(unsigned int *Index[]);
unsigned int MemoryFreeOfSortTwo(unsigned int *Index[]);

// 用于保存Flag、Cigar及比对坐标等信息（Bam行数）；
unsigned int MemoryRequireOfInfo4MapFull(unsigned int ArraySize,unsigned char *GeneralPS4Char[],unsigned short *GeneralPS4Short[],unsigned int *GeneralPS4Int[]);
unsigned int MemoryFreeOfCigar(unsigned short *Info4MapFullCigar);
unsigned int MemoryFreeOfInfo4MapFull(unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos);

// 用于保存Flag、Cigar及比对坐标等信息（Reads对数）；
unsigned int MemoryRequireOfInfo4Map(unsigned int ArraySize,unsigned char *GeneralPS4Char[],unsigned int *GeneralPS4Int[]);
unsigned int MemoryFreeOfInfo4Map(unsigned char *Info4MapLeftChr,unsigned int *Info4MapLeftPos,unsigned char *Info4MapRightChr,unsigned int *Info4MapRightPos);

// 用于存储Index信息；
unsigned int MemoryRequireOfIndex(unsigned int ArraySize,unsigned char *GeneralPS4Char[]);

// 用于保存比对数据的分组信息（Reads对）；
unsigned int MemoryRequireOfMapGroup(unsigned int ArraySize,unsigned int *GeneralPS4Int[]);
unsigned int MemoryFreeOfMapGroup(unsigned int *GroupInfo4Map);

// 用于保存比对数据的分组信息（Bam行数）；
unsigned int MemoryRequireOfMapGroupFull(unsigned int ArraySize,unsigned int *GeneralPS4Int[]);

// 用于记录行偏移；
unsigned int MemoryRequireOfInfo4Shift(unsigned int ArraySize,unsigned long *GeneralPS4Long[]);

#endif
