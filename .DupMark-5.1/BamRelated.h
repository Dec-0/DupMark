#ifndef _BAMRELATED_H
#define _BAMRELATED_H

#include <stdio.h>
#include <stdlib.h>

// 获取sam中前100行的最大reads长度;
unsigned int ReadLenConfirmFromSam(unsigned char *File);

// 确定reads'ID的格式，比如Illumina、BGI或其它格式;
unsigned int IDFormatConfirmFromSam(unsigned char *File);

// 获得Sam头信息；
unsigned int HeadAquireFromSam(unsigned char *File,unsigned char *HeadString);

// 从sam文件，确认UMI相关信息，比如序列、长度等;
unsigned int UmiSizeConfirm(unsigned char *File,unsigned char *UmiPrefix);

// Group info for reads re-mapping;
unsigned int RGConfirm(unsigned char *File, unsigned char *RG);

// 获取ReadsID信息（暂时没有用到IDFormatFlag信息）；
unsigned int InfoGet4ReadsID(unsigned char *SamFile,unsigned int IDFormatFlag,unsigned long *Info4IDFull);

// 用于根据flag标记和Cigar序列确定插入片段的边界；
unsigned short CigarShiftConfirm(unsigned int Flag, unsigned char *Cigar);

// 获取每条reads记录对应的比对信息；
unsigned int InfoGet4Map(unsigned char *SamFile,unsigned short *Info4MapFullCigar,unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos);

// 逐个处理Reads对，定位原始配对Reads对，并统一标记比对信息；
unsigned int MapInfoUniform(unsigned int MapReadsNum,unsigned int *SortInfo4ReadsID,unsigned int *GroupInfo4ReadsID,unsigned short *Info4MapFullCigar,unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos,unsigned int DebugFlag);

// 用于转移Index信息记录；
unsigned int InfoRecord4Index(unsigned int MapReadsNum,unsigned char *Info4Index,unsigned int *SortInfo4ReadsID,unsigned int *GroupInfo4ReadsID,unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos);

// 用于将标记的比对分组信息，从Reads对拓展多所有单条reads；
unsigned int ExpandMapGroup(unsigned int *GroupInfo4ReadsID,unsigned int *GroupInfo4Map,unsigned int *GroupInfo4MapFull,unsigned int MapReadsNum);

// 用于获得文件中每行的偏移量；
unsigned int InfoGet4Shift(unsigned char *SamFile,unsigned long *Info4Shift);

// 用于获得特定行；
unsigned int SpecificLineGet(FILE *fid,unsigned long Shift,unsigned char *Buff);

// 用于分隔行，得到每列的截取数据；
unsigned int ColSplit(unsigned char *Buff,unsigned int LineStart,unsigned int LineEnd,unsigned char Items[][500]);

// 用于获取Reads对对应的碱基之和
unsigned int TotalBaseQualityGetFromSam(FILE *fid,unsigned int *SortInfo4MapFull,unsigned long *Info4Shift,unsigned int Id4Read1,unsigned int Id4Read2);

// 用于修改碱基位；
unsigned int FlagRevise(unsigned char *InFile,unsigned char *OutFile,unsigned char *DupFlag);
unsigned int LineShow(FILE *fid,unsigned int *SortInfo4MapFull,unsigned long *Info4Shift,unsigned int Id,unsigned int GroupId);

// 获得UMI序列信息；
unsigned long UMISeqGetFromSam(FILE *fid,unsigned int *SortInfo4MapFull,unsigned long *Info4Shift,unsigned int Id4Read,unsigned char *ColPrefixOfUMI,unsigned int HUmiSize);

#endif
