#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "SortRelated.h"
#include "BasicRelated.h"

// 用来对单一数组（Int或者Long类型）进行排序，并确认排序和分类结果；
unsigned int SortAndGroup4Int(unsigned int *Info,unsigned int *Index[],unsigned int Num)
{
	unsigned int GroupId;
	unsigned long MaxDupSpan,DulSpan,MinSpan;
	unsigned long LeftBegin,LeftEnd,RightBegin,RightEnd;
	unsigned long i,j,Id1,Id2,tId,tmp,PreInfo;
	Id1 = 0;
	Id2 = 1;
	
	// 初始顺序;
	for(i = 0;i < Num;i ++)
	{
		*(Index[Id1] + i) = i;
	}
	// 从2开始，每轮乘以2逐渐翻倍;
	MaxDupSpan = Num;
	MaxDupSpan = MaxDupSpan << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tId = 0;
		
		// 比如从0，2，4...开始；
		for(i = 0;i < Num;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < Num)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= Num)
				{
					RightEnd = Num - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					// 单纯比大小，大靠后；
					else if(*(Info + *(Index[Id1] + LeftBegin)) > *(Info + *(Index[Id1] + RightBegin)))
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < Num;j ++)
				{
					*(Index[Id2] + tId) = *(Index[Id1] + j);
					tId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	
	// Index[0]记录排序；
	if(Id1 == 1)
	{
		for(i = 0;i < Num;i ++)
		{
			*(Index[0] + i) = *(Index[1] + i);
		}
	}
	// Index[1]记录分组；
	GroupId = 0;
	// 应该没有哪个ReadsID是这种吧，0倒是有可能；
	PreInfo = 0xffffffff;
	for(i = 0;i < Num;i ++)
	{
		if(*(Info + *(Index[0] + i)) != PreInfo)
		{
			GroupId ++;
			PreInfo = *(Info + *(Index[0] + i));
		}
		
		*(Index[1] + *(Index[0] + i)) = GroupId;
	}
	
	return GroupId;
}
unsigned int SortAndGroup4Long(unsigned long *Info,unsigned int *Index[],unsigned int Num)
{
	unsigned int GroupId;
	unsigned long MaxDupSpan,DulSpan,MinSpan;
	unsigned long LeftBegin,LeftEnd,RightBegin,RightEnd;
	unsigned long i,j,Id1,Id2,tId,tmp,PreInfo;
	Id1 = 0;
	Id2 = 1;
	
	// 初始顺序;
	for(i = 0;i < Num;i ++)
	{
		*(Index[Id1] + i) = i;
	}
	// 从2开始，每轮乘以2逐渐翻倍;
	MaxDupSpan = Num;
	MaxDupSpan = MaxDupSpan << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tId = 0;
		
		// 比如从0，2，4...开始；
		for(i = 0;i < Num;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < Num)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= Num)
				{
					RightEnd = Num - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					// 单纯比大小，大靠后；
					else if(*(Info + *(Index[Id1] + LeftBegin)) > *(Info + *(Index[Id1] + RightBegin)))
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < Num;j ++)
				{
					*(Index[Id2] + tId) = *(Index[Id1] + j);
					tId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	
	// Index[0]记录排序；
	if(Id1 == 1)
	{
		for(i = 0;i < Num;i ++)
		{
			*(Index[0] + i) = *(Index[1] + i);
		}
	}
	// Index[1]记录分组，从编号1开始；
	GroupId = 0;
	// 应该没有哪个ReadsID是这种吧，0倒是有可能；
	PreInfo = 0xffffffffffffffff;
	for(i = 0;i < Num;i ++)
	{
		if(*(Info + *(Index[0] + i)) != PreInfo)
		{
			GroupId ++;
			PreInfo = *(Info + *(Index[0] + i));
		}
		
		*(Index[1] + *(Index[0] + i)) = GroupId;
	}
	
	return GroupId;
}

// 对Mapping Info进行规范化处理
unsigned int MapInfoStandard(unsigned char *Chr,unsigned int *Pos,unsigned char *PChr,unsigned int *PPos,unsigned int Num,unsigned int Flag)
{
	unsigned char tChr;
	unsigned int i,tPos;
	
	for(i = 0;i < Num;i ++)
	{
		if(*(Chr + i) == 0 || *(PChr + i) == 0)
		{
			// 永远是 非0 + 0 的模式；
			if(*(PChr + i) != 0)
			{
				tChr = *(Chr + i);
				tPos = *(Pos + i);
				*(Chr + i) = *(PChr + i);
				*(Pos + i) = *(PPos + i);
				*(PChr + i) = tChr;
				*(PPos + i) = tPos;
			}
			// 后为0也就没有再比较的必要了；
			continue;
		}
		
		// 小在前大在后；
		if(Flag == 0)
		{
			if(*(Chr + i) > *(PChr + i))
			{
				tChr = *(Chr + i);
				tPos = *(Pos + i);
				*(Chr + i) = *(PChr + i);
				*(Pos + i) = *(PPos + i);
				*(PChr + i) = tChr;
				*(PPos + i) = tPos;
			}
			else if(*(Chr + i) == *(PChr + i) && *(Pos + i) > *(PPos + i))
			{
				tChr = *(Chr + i);
				tPos = *(Pos + i);
				*(Chr + i) = *(PChr + i);
				*(Pos + i) = *(PPos + i);
				*(PChr + i) = tChr;
				*(PPos + i) = tPos;
			}
		}
		// 大在前小在后；
		else
		{
			if(*(Chr + i) < *(PChr + i))
			{
				tChr = *(Chr + i);
				tPos = *(Pos + i);
				*(Chr + i) = *(PChr + i);
				*(Pos + i) = *(PPos + i);
				*(PChr + i) = tChr;
				*(PPos + i) = tPos;
			}
			else if(*(Chr + i) == *(PChr + i) && *(Pos + i) < *(PPos + i))
			{
				tChr = *(Chr + i);
				tPos = *(Pos + i);
				*(Chr + i) = *(PChr + i);
				*(Pos + i) = *(PPos + i);
				*(PChr + i) = tChr;
				*(PPos + i) = tPos;
			}
		}
	}
	
	return 1;
}

// 比较坐标大小；
unsigned int MapInfoCompare(unsigned char *Chr,unsigned int *Pos,unsigned char *PChr,unsigned int *PPos,unsigned int LeftBegin,unsigned int RightBegin)
{
	// 需要两个long分别记录匹配坐标和配对的匹配坐标；
	unsigned int CompareFlag = 0;
	unsigned long LeftValue,LeftPValue,RightValue,RightPValue;
	
	LeftValue = *(Chr + LeftBegin);
	LeftValue = LeftValue << 32 | *(Pos + LeftBegin);
	LeftPValue = *(PChr + LeftBegin);
	LeftPValue = LeftPValue << 32 | *(PPos + LeftBegin);
	RightValue = *(Chr + RightBegin);
	RightValue = RightValue << 32 | *(Pos + RightBegin);
	RightPValue = *(PChr + RightBegin);
	RightPValue = RightPValue << 32 | *(PPos + RightBegin);
	
	// 含有非0的一定是非0在Left，此时需要直接比较大小；
	// 含有非0的不能和全0的分在一组；
	// 部分非0的需要额外容错比较，a0需要排在aa后面；
	// 主大小就是严格按照大小排序，次大小是指需要标记aa和a0一样，总的来说含有0x20的就是主大，含有0x10的就是主小，含有0x02或者0x01的就是次大小；
	// 主大小能一次性判断，次大小也能一次性判断；
	if(LeftValue > RightValue)
	{
		return 0x22;
	}
	else if(LeftValue < RightValue)
	{
		return 0x11;
	}
	
	// Right的大小在非0时代表主大小，而在含0时代表的是次（容错）大小；
	if(LeftPValue > RightPValue)
	{
		if(RightPValue)
		{
			return 0x22;
		}
		else
		{
			return 0x20;
		}
	}
	else if(LeftPValue < RightPValue)
	{
		if(LeftPValue)
		{
			return 0x11;
		}
		else
		{
			return 0x10;
		}
	}
	
	return CompareFlag;
}

// 用来对比对数据进行排序，返回排序和分类结果；
unsigned int SortAndGroup4MapInfo(time_t start,unsigned char *Chr,unsigned int *Pos,unsigned char *PChr,unsigned int *PPos,unsigned int *Index[],unsigned int *Group,unsigned int Num,unsigned int DebugFlag)
{
	unsigned int GroupId;
	unsigned long MaxDupSpan,DulSpan,MinSpan;
	unsigned long LeftBegin,LeftEnd,RightBegin,RightEnd;
	unsigned long i,j,Id1,Id2,tId,tmp,OriFlag,From,To;
	Id1 = 0;
	Id2 = 1;
	
	memset(Group,0,Num * sizeof(unsigned int));
	
	// 在格式化顺序时，将小的放前面，大的放后面，0+X配对时X自动放在前面；
	OriFlag = 0;
	MapInfoStandard(Chr,Pos,PChr,PPos,Num,OriFlag);
	TimeLog(start,"Min ahead max behind done");
	for(i = 0;i < Num;i ++)
	{
		*(Index[Id1] + i) = i;
	}
	// 2,4,8,16.....;
	MaxDupSpan = Num;
	MaxDupSpan = MaxDupSpan << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tId = 0;
		
		// 比如从0，2，4...开始；
		for(i = 0;i < Num;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < Num)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= Num)
				{
					RightEnd = Num - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					// 在排序时最好大前小后，毕竟在分类编号时以第一个的编号为准。不然a0、ab、ac可能就分为一组了，这样会导致组成员集合变得很大。所以一定要是信息完整的条目在前面。
					else if(MapInfoCompare(Chr,Pos,PChr,PPos,*(Index[Id1] + LeftBegin),*(Index[Id1] + RightBegin)) & 0x10)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < Num;j ++)
				{
					*(Index[Id2] + tId) = *(Index[Id1] + j);
					tId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	// 记录分组；
	From = 0;
	GroupId = 1;
	while(From < Num)
	{
		for(To = From + 1;To < Num;To ++)
		{
			// 0x00中第1位指绝对大小，第2位指容错大小，第2位为0时，第1位不一定能是0；
			if(MapInfoCompare(Chr,Pos,PChr,PPos,*(Index[Id1] + To),*(Index[Id1] + From)) & 0x0f)
			{
				break;
			}
		}
		
		for(i = From;i < To;i ++)
		{
			*(Group + *(Index[Id1] + i)) = GroupId;
		}
		
		if(DebugFlag)
		{
			printf("\n");
			for(i = From;i < To;i ++)
			{
				printf("[ MapInfoSort1 ] %u %x - %u %x  %u\n",*(Chr + *(Index[Id1] + i)),*(Pos + *(Index[Id1] + i)),*(PChr + *(Index[Id1] + i)),*(PPos + *(Index[Id1] + i)),*(Group + *(Index[Id1] + i)));
			}
		}
		
		From = To;
		GroupId ++;
	}
	TimeLog(start,"Sort done for min ahead max behind");
	
	// 将大的放前面，小的放后面，0+X配对时X自动放在前面；
	OriFlag = 1;
	MapInfoStandard(Chr,Pos,PChr,PPos,Num,OriFlag);
	TimeLog(start,"Max ahead min behind done");
	// 2,4,8,16.....;
	MaxDupSpan = Num;
	MaxDupSpan = MaxDupSpan << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tId = 0;
		
		// 比如从0，2，4...开始；
		for(i = 0;i < Num;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < Num)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= Num)
				{
					RightEnd = Num - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					else if(MapInfoCompare(Chr,Pos,PChr,PPos,*(Index[Id1] + LeftBegin),*(Index[Id1] + RightBegin)) & 0x10)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < Num;j ++)
				{
					*(Index[Id2] + tId) = *(Index[Id1] + j);
					tId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	// 记录分组；
	From = 0;
	while(From < Num)
	{
		for(To = From + 1;To < Num;To ++)
		{
			if(MapInfoCompare(Chr,Pos,PChr,PPos,*(Index[Id1] + To),*(Index[Id1] + From)) & 0x0f)
			{
				break;
			}
		}
		
		if(!*(Chr + *(Index[Id1] + From)) && !*(PChr + *(Index[Id1] + From)))
		{
			// 双链均未比对上的，分组编号置为0；
			for(i = From;i < To;i ++)
			{
				*(Group + *(Index[Id1] + i)) = 0;
			}
		}
		else
		{
			for(i = From + 1;i < To;i ++)
			{
				*(Group + *(Index[Id1] + i)) = *(Group + *(Index[Id1] + From));
			}
			
			if(DebugFlag)
			{
				printf("\n");
				for(i = From;i < To;i ++)
				{
					printf("[ MapInfoSort2 ] %u %x - %u %x  %u\n",*(Chr + *(Index[Id1] + i)),*(Pos + *(Index[Id1] + i)),*(PChr + *(Index[Id1] + i)),*(PPos + *(Index[Id1] + i)),*(Group + *(Index[Id1] + i)));
				}
			}
		}
		
		From = To;
	}
	TimeLog(start,"Sort done for max ahead min behind");
	
	return 1;
}

// 给定一个完整的全序列参考，在限定范围内再排序；
unsigned int ReSortByString(unsigned int From,unsigned int To,unsigned int *SortInfo,unsigned int *GroupInfo)
{
	unsigned int Index[2][100000];
	unsigned int MaxDupSpan,DulSpan,MinSpan;
	unsigned int LeftBegin,LeftEnd,RightBegin,RightEnd;
	unsigned int i,j,Id1,Id2,tId,tmp,Num;
	Id1 = 0;
	Id2 = 1;
	
	// 初始顺序（左闭右开）;
	Num = To - From;
	for(i = 0;i < Num;i ++)
	{
		*(Index[Id1] + i) = *(SortInfo + From + i);
	}
	// 从2开始，每轮乘以2逐渐翻倍;
	MaxDupSpan = Num;
	MaxDupSpan = MaxDupSpan << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tId = 0;
		
		// 比如从0，2，4...开始；
		for(i = 0;i < Num;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < Num)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= Num)
				{
					RightEnd = Num - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					// 单纯比大小，大靠后；
					else if(*(GroupInfo + *(Index[Id1] + LeftBegin)) > *(GroupInfo + *(Index[Id1] + RightBegin)))
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < Num;j ++)
				{
					*(Index[Id2] + tId) = *(Index[Id1] + j);
					tId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	
	// Index[0]记录排序；
	for(i = 0;i < Num;i ++)
	{
		*(SortInfo + From + i) = *(Index[Id1] + i);
	}
	
	return Num;
}

// UMI序列对比（需要区分单侧和双侧的区别）；
unsigned int UMISeqCompare(unsigned long UMIA,unsigned long UMIB,unsigned int HUmiSize)
{
	unsigned int i,Shift,tA,tB;
	unsigned int MatchFlag = 1;
	unsigned long UMIHalfB[2];
	unsigned long UMIBr;
	
	// 假如是单端UMI；
	if(! HUmiSize)
	{
		if(UMIA != UMIB)
		{
			MatchFlag = 0;
		}
		return MatchFlag;
	}
	
	UMIHalfB[0] = UMIB & 0x3fffffff;
	UMIHalfB[1] = UMIB >> 30 & 0x3fffffff;
	UMIBr = UMIHalfB[0] << 30 | UMIHalfB[1];
	
	for(i = 0;i < 20;i ++)
	{
		Shift = i * 3;
		tA = (UMIA >> Shift) & 0x7;
		tB = (UMIB >> Shift) & 0x7;
		// 非N时的比较才有意义
		if(tA > 0 && tB > 0 && tA != tB)
		{
			MatchFlag = 0;
			break;
		}
	}
	
	if(!MatchFlag)
	{
		MatchFlag = 2;
		for(i = 0;i < 20;i ++)
		{
			Shift = i * 3;
			tA = (UMIA >> Shift) & 0x7;
			tB = (UMIBr >> Shift) & 0x7;
			// 非N时的比较才有意义
			if(tA > 0 && tB > 0 && tA != tB)
			{
				MatchFlag = 0;
				break;
			}
		}
	}
	
	return MatchFlag;
}
/*
UMI序列中的N需要考虑进去。
unsigned int ReSortByUMI(unsigned int PairNum,unsigned int *PairFrom,unsigned int *PairTo,unsigned int *Read1,unsigned int *Read2,unsigned long *UMISeq,unsigned char *FRFlag)
{
	unsigned char ArrayChar[100000];
	unsigned char tmpChar;
	unsigned int Index[2][100000],ArrayInt[100000];
	unsigned int MaxDupSpan,DulSpan,MinSpan;
	unsigned int LeftBegin,LeftEnd,RightBegin,RightEnd;
	unsigned int i,j,Id1,Id2,tId,tmp,Num,tmpInt;
	unsigned long tmpLong;
	unsigned long ArrayLong[100000];
	Id1 = 0;
	Id2 = 1;
	
	// 初始顺序（左闭右开）;
	Num = PairNum;
	for(i = 0;i < Num;i ++)
	{
		*(Index[Id1] + i) = i;
	}
	// 从2开始，每轮乘以2逐渐翻倍;
	MaxDupSpan = Num;
	MaxDupSpan = MaxDupSpan << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tId = 0;
		
		// 比如从0，2，4...开始；
		for(i = 0;i < Num;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < Num)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= Num)
				{
					RightEnd = Num - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					// 单纯比大小，大靠后；
					else if(*(UMISeq + LeftBegin) > *(UMISeq + RightBegin))
					{
						*(Index[Id2] + tId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < Num;j ++)
				{
					*(Index[Id2] + tId) = *(Index[Id1] + j);
					tId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	
	// Real Part；
	for(i = 0;i < Num;i ++)
	{
		ArrayInt[i] = *(PairFrom + *(Index[Id1] + i));
	}
	for(i = 0;i < Num;i ++)
	{
		*(PairFrom + i) = ArrayInt[i];
	}
	
	for(i = 0;i < Num;i ++)
	{
		ArrayInt[i] = *(PairTo + *(Index[Id1] + i));
	}
	for(i = 0;i < Num;i ++)
	{
		*(PairTo + i) = ArrayInt[i];
	}
	
	for(i = 0;i < Num;i ++)
	{
		ArrayInt[i] = *(Read1 + *(Index[Id1] + i));
	}
	for(i = 0;i < Num;i ++)
	{
		*(Read1 + i) = ArrayInt[i];
	}
	
	for(i = 0;i < Num;i ++)
	{
		ArrayInt[i] = *(Read2 + *(Index[Id1] + i));
	}
	for(i = 0;i < Num;i ++)
	{
		*(Read2 + i) = ArrayInt[i];
	}
	
	for(i = 0;i < Num;i ++)
	{
		ArrayLong[i] = *(UMISeq + *(Index[Id1] + i));
	}
	for(i = 0;i < Num;i ++)
	{
		*(UMISeq + i) = ArrayLong[i];
	}
	
	for(i = 0;i < Num;i ++)
	{
		ArrayChar[i] = *(FRFlag + *(Index[Id1] + i));
	}
	for(i = 0;i < Num;i ++)
	{
		*(FRFlag + i) = ArrayChar[i];
	}
	
	return Num;
}
*/
unsigned int ReSortByUMI(unsigned int PairNum,unsigned int *PairFrom,unsigned int *PairTo,unsigned int *Read1,unsigned int *Read2,unsigned long *UMISeq,unsigned int HUmiSize,unsigned char *FRFlag)
{
	unsigned char BlankFlag[100000],tFRFlag[100000];
	unsigned int tPairFrom[100000],tPairTo[100000],tRead1[100000],tRead2[100000];
	unsigned int i,j,tId,MatchFlag;
	unsigned long tUMISeq[100000];
	
	// 标记是否被处理过；
	memset(BlankFlag,1,PairNum * sizeof(unsigned char));
	tId = 0;
	for(i = 0;i < PairNum;i ++)
	{
		// 处理过的不再处理；
		if(!BlankFlag[i])
		{
			continue;
		}
		
		tPairFrom[tId] = *(PairFrom + i);
		tPairTo[tId] = *(PairTo + i);
		tRead1[tId] = *(Read1 + i);
		tRead2[tId] = *(Read2 + i);
		tUMISeq[tId] = *(UMISeq + i);
		tFRFlag[tId] = 1;
		tId += 1;
		BlankFlag[i] = 0;
		for(j = i + 1;j < PairNum;j ++)
		{
			if(BlankFlag[j])
			{
				MatchFlag = UMISeqCompare(*(UMISeq + i),*(UMISeq + j),HUmiSize);
				if(MatchFlag > 0)
				{
					tPairFrom[tId] = *(PairFrom + j);
					tPairTo[tId] = *(PairTo + j);
					tRead1[tId] = *(Read1 + j);
					tRead2[tId] = *(Read2 + j);
					tUMISeq[tId] = *(UMISeq + j);
					tFRFlag[tId] = MatchFlag;
					tId += 1;
					BlankFlag[j] = 0;
				}
			}
		}
	}
	
	for(i = 0;i < PairNum;i ++)
	{
		*(PairFrom + i) = tPairFrom[i];
		*(PairTo + i) = tPairTo[i];
		*(Read1 + i) = tRead1[i];
		*(Read2 + i) = tRead2[i];
		*(UMISeq + i) = tUMISeq[i];
		*(FRFlag + i) = tFRFlag[i];
	}
	
	return PairNum;
}