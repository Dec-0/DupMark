#include <stdio.h>
#include <stdlib.h>
#include "MemoryRelated.h"

// 分配Bam行数 * 8 Bytes的空间，用于存储ReadsID信息;
unsigned int MemoryRequireOfInfo4IDFull(unsigned int ArraySize,unsigned long *GeneralPS4Long[])
{
	if((*(GeneralPS4Long + 0) = (unsigned long *)malloc(ArraySize * sizeof(unsigned long))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4IDFull %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}
unsigned int MemoryFreeOfInfo4IDFull(unsigned long *Info4IDFull)
{
	free(Info4IDFull);
	return 1;
}

unsigned int MemoryFreeOfSortInfo4ReadsID(unsigned int *SortInfo4ReadsID)
{
	free(SortInfo4ReadsID);
	return 1;
}

// 一个二维数组，专门用于排序；
unsigned int MemoryRequireOfSort(unsigned int ArraySize,unsigned int *Index4Int[])
{
	unsigned char i;
	
	for(i = 0;i < 2;i ++)
	{
		if((Index4Int[i] = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
		{
			printf("[ Error ] Malloc memory unsuccessfully ( Index4Int%d %u).\n",i,ArraySize);
			exit(1);
		}
	}
	
	return 1;
}
unsigned int MemoryFreeOfSortBase(unsigned int *Index[])
{
	free(Index);
	return 1;
}
unsigned int MemoryFreeOfSortFull(unsigned int *Index[])
{
	free(Index[0]);
	free(Index[1]);
	free(Index);
	return 1;
}
unsigned int MemoryFreeOfSortOne(unsigned int *Index[])
{
	free(Index[0]);
	free(Index);
	return 1;
}
unsigned int MemoryFreeOfSortTwo(unsigned int *Index[])
{
	free(Index[1]);
	free(Index);
	return 1;
}

// 用于保存Flag、Cigar及比对坐标等信息（Bam行数）；
unsigned int MemoryRequireOfInfo4MapFull(unsigned int ArraySize,unsigned char *GeneralPS4Char[],unsigned short *GeneralPS4Short[],unsigned int *GeneralPS4Int[])
{
	if((*(GeneralPS4Short + 0) = (unsigned short *)malloc(ArraySize * sizeof(unsigned short))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapFullCigar %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Char + 0) = (unsigned char *)malloc(ArraySize * sizeof(unsigned char))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapFullLeftChr %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Int + 0) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapFullLeftPos %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Char + 1) = (unsigned char *)malloc(ArraySize * sizeof(unsigned char))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapFullRightChr %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Int + 1) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapFullRightPos %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

unsigned int MemoryFreeOfCigar(unsigned short *Info4MapFullCigar)
{
	free(Info4MapFullCigar);
	
	return 1;
}

unsigned int MemoryFreeOfInfo4MapFull(unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos)
{
	free(Info4MapFullLeftChr);
	free(Info4MapFullLeftPos);
	free(Info4MapFullRightChr);
	free(Info4MapFullRightPos);
	
	return 1;
}

// 用于保存Flag、Cigar及比对坐标等信息（Reads对数）；
unsigned int MemoryRequireOfInfo4Map(unsigned int ArraySize,unsigned char *GeneralPS4Char[],unsigned int *GeneralPS4Int[])
{
	if((*(GeneralPS4Char + 0) = (unsigned char *)malloc(ArraySize * sizeof(unsigned char))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapLeftChr %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Int + 0) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapLeftPos %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Char + 1) = (unsigned char *)malloc(ArraySize * sizeof(unsigned char))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapRightChr %u).\n",ArraySize);
		exit(1);
	}
	
	if((*(GeneralPS4Int + 1) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4MapRightPos %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

unsigned int MemoryFreeOfInfo4Map(unsigned char *Info4MapLeftChr,unsigned int *Info4MapLeftPos,unsigned char *Info4MapRightChr,unsigned int *Info4MapRightPos)
{
	free(Info4MapLeftChr);
	free(Info4MapLeftPos);
	free(Info4MapRightChr);
	free(Info4MapRightPos);
	
	return 1;
}

// 用于存储Index信息；
unsigned int MemoryRequireOfIndex(unsigned int ArraySize,unsigned char *GeneralPS4Char[])
{
	if((*(GeneralPS4Char + 0) = (unsigned char *)malloc(ArraySize * sizeof(unsigned char))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4Index %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

// 用于保存比对数据的分组信息（Reads对）；
unsigned int MemoryRequireOfMapGroup(unsigned int ArraySize,unsigned int *GeneralPS4Int[])
{
	if((*(GeneralPS4Int + 0) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( MapGroup %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

unsigned int MemoryFreeOfMapGroup(unsigned int *GroupInfo4Map)
{
	free(GroupInfo4Map);
	
	return 1;
}

// 用于保存比对数据的分组信息（Bam行数）；
unsigned int MemoryRequireOfMapGroupFull(unsigned int ArraySize,unsigned int *GeneralPS4Int[])
{
	if((*(GeneralPS4Int + 0) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( MapGroupFull %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

// 用于记录行偏移；
unsigned int MemoryRequireOfInfo4Shift(unsigned int ArraySize,unsigned long *GeneralPS4Long[])
{
	if((*(GeneralPS4Long + 0) = (unsigned long *)malloc(ArraySize * sizeof(unsigned long))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4Shift %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

// 用于记录UMI序列信息；
unsigned int MemoryRequireOfInfo4UMISeq(unsigned int ArraySize,unsigned long *GeneralPS4Long[])
{
	if((*(GeneralPS4Long + 0) = (unsigned long *)malloc(ArraySize * sizeof(unsigned long))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4UMISeq %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}

// 用于记录质量信息；
unsigned int MemoryRequireOfInfo4BQ(unsigned int ArraySize,unsigned int *GeneralPS4Int[])
{
	if((*(GeneralPS4Int + 0) = (unsigned int *)malloc(ArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Info4BQ %u).\n",ArraySize);
		exit(1);
	}
	
	return 1;
}