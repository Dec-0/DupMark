#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ConsensusRelated.h"
#include "BasicRelated.h"
#include "BamRelated.h"
#include "SortRelated.h"

// 非UMI条件下的标记重复；
unsigned int PairReadsNumConfirm(unsigned int From,unsigned int To,unsigned int *SortInfo,unsigned int *GroupInfo)
{
	unsigned int tFrom,tTo;
	unsigned int PairNum = 0;
	
	tFrom = From;
	while(tFrom < To)
	{
		for(tTo = tFrom + 1;tTo < To;tTo ++)
		{
			if(*(GroupInfo + *(SortInfo + tTo)) != *(GroupInfo + *(SortInfo + tFrom)))
			{
				break;
			}
		}
		
		PairNum ++;
		tFrom = tTo;
	}
	
	return PairNum;
}
unsigned int PairReadsConfirm(unsigned int From,unsigned int To,unsigned int *SortInfo,unsigned int *GroupInfo,unsigned char *Info4Index,unsigned int *PairFrom,unsigned int *PairTo,unsigned int *Read1,unsigned int *Read2)
{
	unsigned int tFrom,tTo,i,ReadFlag;
	unsigned int FRNum = 0;
	unsigned int RFNum = 0;
	unsigned int PairNum = 0;
	
	tFrom = From;
	while(tFrom < To)
	{
		for(tTo = tFrom + 1;tTo < To;tTo ++)
		{
			if(*(GroupInfo + *(SortInfo + tTo)) != *(GroupInfo + *(SortInfo + tFrom)))
			{
				break;
			}
		}
		PairFrom[PairNum] = tFrom;
		PairTo[PairNum] = tTo - 1;
		
		// 确定原始的Read1和Read2；
		ReadFlag = 0;
		for(i = tFrom;i < tTo;i ++)
		{
			// Read1;
			if(*(Info4Index + *(SortInfo + i)) & 0x01)
			{
				Read1[PairNum] = i;
				ReadFlag += 1;
			}
			// Read2;
			else if(*(Info4Index + *(SortInfo + i)) & 0x02)
			{
				Read2[PairNum] = i;
				ReadFlag += 2;
			}
			
			if(*(Info4Index + *(SortInfo + i)) & 0x10)
			{
				FRNum += 1;
			}
			else if(*(Info4Index + *(SortInfo + i)) & 0x20)
			{
				RFNum += 1;
			}
		}
		// 假如没有碰到Read1 + Read2，需要报错；
		if(ReadFlag != 3)
		{
			printf("[ Error ] Could not locate Read1 + Read2  (%u) in PairReadsConfirm (No.%u).\n",ReadFlag,PairNum);
			exit(1);
		}
		
		PairNum ++;
		tFrom = tTo;
	}
	
	if(FRNum + RFNum != PairNum)
	{
		printf("[ Error ] FRNum + RFNum != PairNum.\n");
		exit(1);
	}
	
	return PairNum;
}
unsigned int DupMarkWithOutUMI(time_t start,unsigned char *SamFile,unsigned char *OutSamFile,unsigned int MapReadsNum,unsigned long *Info4Shift,unsigned int *GroupInfo4ReadsID,unsigned int *SortInfo4MapFull,unsigned int *GroupInfo4MapFull,unsigned char *Info4Index,unsigned int BamGenJumpFlag,unsigned char *DupLogPath)
{
	FILE *fid,*fdlp;
	// 注意unmap的数量可能会超过这个数值;
	unsigned char DupFlag[100000],cmd[1000],LogBuffContent[100000];
	unsigned int tBQ,MaxBQ;
	unsigned int PairFrom[100000],PairTo[100000],Read1[100000],Read2[100000];
	unsigned int From,To,PairNum,MaxId,i,j,AccumCheckPoint,AccumCheckMulti,CheckNum,FRNum,RFNum;
	unsigned int MaxLogBuffSize = 80000;
	unsigned int CurrLogBuffSize = 0;
	
	// 用于记录Reads重复信息；
	fdlp = fopen(DupLogPath,"w");
	if(fdlp == NULL)
	{
		printf("File cannot be open: %s\n",DupLogPath);
		exit(1);
	}
	LogBuffContent[CurrLogBuffSize] = '\0';
	if(!BamGenJumpFlag)
	{
		// 按比对位置逐个处理，挑选总碱基质量最高的Reads对，进行标记并输出；
		fid = fopen(SamFile,"r");
		if(fid == NULL)
		{
			printf("File cannot be open: %s\n",SamFile);
			exit(1);
		}
	}
	FRNum = 0;
	RFNum = 0;
	From = 0;
	AccumCheckMulti = 1000000;
	CheckNum = 1;
	AccumCheckPoint = AccumCheckMulti * CheckNum;
	while(From < MapReadsNum)
	{
		// 选取比对信息一致的集合;
		for(To = From + 1;To < MapReadsNum;To ++)
		{
			if(*(GroupInfo4MapFull + *(SortInfo4MapFull + To)) != *(GroupInfo4MapFull + *(SortInfo4MapFull + From)))
			{
				break;
			}
		}
		
		// 按ReadsID顺序对SortInfo4MapFull的顺序进行处理；
		ReSortByString(From,To,SortInfo4MapFull,GroupInfo4ReadsID);
		// 假如都是un-map；
		if(*(GroupInfo4MapFull + *(SortInfo4MapFull + From)) == 0)
		{
			// 确定Reads对数;
			PairNum = PairReadsNumConfirm(From,To,SortInfo4MapFull,GroupInfo4ReadsID);
			// 都是非重复，将DupFlag信息保存到Info4Index；
			sprintf(cmd,"Matching un-map %u reads' pair (%u - %u)",PairNum,From,To);
			TimeLog(start,cmd);
			
			From = To;
			continue;
		}
		
		// 确定Reads对个数，以及对应的原始Read1、Read2编号；
		PairNum = PairReadsConfirm(From,To,SortInfo4MapFull,GroupInfo4ReadsID,Info4Index,PairFrom,PairTo,Read1,Read2);
		// 统计FR和RF数量；
		FRNum = 0;
		RFNum = 0;
		for(i = From;i < To;i ++)
		{
			if(*(Info4Index + *(SortInfo4MapFull + i)) & 0x10)
			{
				FRNum += 1;
			}
			else if(*(Info4Index + *(SortInfo4MapFull + i)) & 0x20)
			{
				RFNum += 1;
			}
			// 用于Debug LineShow(fid,SortInfo4MapFull,Info4Shift,i,*(GroupInfo4MapFull + *(SortInfo4MapFull + i)));
		}
		// 记录TagSizeFamily信息；
		CurrLogBuffSize = IntAdd2StringSimple(LogBuffContent,FRNum,CurrLogBuffSize);
		CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'\t',CurrLogBuffSize);
		CurrLogBuffSize = IntAdd2StringSimple(LogBuffContent,RFNum,CurrLogBuffSize);
		CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'\n',CurrLogBuffSize);
		// 标记重复；
		if(!BamGenJumpFlag)
		{
			// 初始化均为Dup；
			memset(DupFlag,1,PairNum * sizeof(unsigned char));
			// 挑选碱基质量和最大的Reads对；
			MaxId = 0;
			MaxBQ = 0;
			
			for(i = 0;i < PairNum;i ++)
			{
				tBQ = TotalBaseQualityGetFromSam(fid,SortInfo4MapFull,Info4Shift,Read1[i],Read2[i]);
				if(tBQ > MaxBQ)
				{
					MaxId = i;
					MaxBQ = tBQ;
				}
			}
			
			DupFlag[MaxId] = 0;
			// 将DupFlag信息保存到Info4Index
			for(i = 0;i < PairNum;i ++)
			{
				for(j = PairFrom[i];j <= PairTo[i];j ++)
				{
					*(Info4Index + *(SortInfo4MapFull + j)) = DupFlag[i];
				}
			}
		}
		
		From = To;
		
		
		// 记录处理量;
		if(From > AccumCheckPoint)
		{
			sprintf(cmd,"Have processed %ld read lines",From);
			TimeLog(start,cmd);
			
			CheckNum ++;
			AccumCheckPoint = AccumCheckMulti * CheckNum;
		}
		if(CurrLogBuffSize > MaxLogBuffSize)
		{
			fwrite(LogBuffContent,1,CurrLogBuffSize,fdlp);
			CurrLogBuffSize = 0;
		}
	}
	if(CurrLogBuffSize)
	{
		fwrite(LogBuffContent,1,CurrLogBuffSize,fdlp);
		CurrLogBuffSize = 0;
	}
	if(!BamGenJumpFlag)
	{
		fclose(fid);
	}
	fclose(fdlp);
	
	return 1;
}


// UMI条件下的去重；
unsigned int DupMarkWithUMI(time_t start,unsigned char *SamFile,unsigned char *OutSamFile,unsigned int MapReadsNum,unsigned long *Info4Shift,unsigned int *GroupInfo4ReadsID,unsigned int *SortInfo4MapFull,unsigned int *GroupInfo4MapFull,unsigned char *Info4Index,unsigned int BamGenJumpFlag,unsigned char *DupLogPath,unsigned char *UmiName,unsigned int HUmiSize,unsigned int FakeFlag)
{
	FILE *fid,*fdlp;
	// 注意unmap的数量可能会超过这个数值;
	unsigned char DupFlag[100000],FRFlag[100000],cmd[1000],LogBuffContent[100000];
	unsigned int PairFrom[100000],PairTo[100000],Read1[100000],Read2[100000];
	unsigned int tBQ,MaxBQ,From,To,UMIFrom,UMITo,PairNum,MaxId,i,j,AccumCheckPoint,AccumCheckMulti,CheckNum,FRNum,RFNum;
	unsigned int MaxLogBuffSize = 80000;
	unsigned int CurrLogBuffSize = 0;
	unsigned long UMISeq[100000];
	
	// 用于记录Reads重复信息；
	fdlp = fopen(DupLogPath,"w");
	if(fdlp == NULL)
	{
		printf("File cannot be open: %s\n",DupLogPath);
		exit(1);
	}
	LogBuffContent[CurrLogBuffSize] = '\0';
	// 按比对位置逐个处理，挑选总碱基质量最高的Reads对，进行标记并输出；
	fid = fopen(SamFile,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",SamFile);
		exit(1);
	}
	FRNum = 0;
	RFNum = 0;
	From = 0;
	AccumCheckMulti = 1000000;
	CheckNum = 1;
	AccumCheckPoint = AccumCheckMulti * CheckNum;
	while(From < MapReadsNum)
	{
		// 选取比对信息一致的集合;
		for(To = From + 1;To < MapReadsNum;To ++)
		{
			if(*(GroupInfo4MapFull + *(SortInfo4MapFull + To)) != *(GroupInfo4MapFull + *(SortInfo4MapFull + From)))
			{
				break;
			}
		}
		
		// 按ReadsID顺序对SortInfo4MapFull的顺序进行处理；
		ReSortByString(From,To,SortInfo4MapFull,GroupInfo4ReadsID);
		// 假如都是un-map；
		if(*(GroupInfo4MapFull + *(SortInfo4MapFull + From)) == 0)
		{
			// 确定Reads对数;
			PairNum = PairReadsNumConfirm(From,To,SortInfo4MapFull,GroupInfo4ReadsID);
			// 都是非重复，将DupFlag信息保存到Info4Index；
			sprintf(cmd,"Matching un-map %u reads' pair (%u - %u)",PairNum,From,To);
			TimeLog(start,cmd);
			
			From = To;
			continue;
		}
		
		// 确定Reads对个数，以及对应的原始Read1、Read2编号；
		PairNum = PairReadsConfirm(From,To,SortInfo4MapFull,GroupInfo4ReadsID,Info4Index,PairFrom,PairTo,Read1,Read2);
		for(i = 0;i < PairNum;i ++)
		{
			UMISeq[i] = UMISeqGetFromSam(fid,SortInfo4MapFull,Info4Shift,Read1[i],UmiName,HUmiSize);
			FRFlag[i] = 0;
			UMISeq[i] &= 0x7fffffffffffffff;
		}
		// 按UMI序列进行排序；
		ReSortByUMI(PairNum,PairFrom,PairTo,Read1,Read2,UMISeq,FRFlag);
		
		// 统计UMI分组统计前整体的FR和RF数量；
		FRNum = 0;
		RFNum = 0;
		for(i = From;i < To;i ++)
		{
			if(*(Info4Index + *(SortInfo4MapFull + i)) & 0x10)
			{
				FRNum += 1;
			}
			else if(*(Info4Index + *(SortInfo4MapFull + i)) & 0x20)
			{
				RFNum += 1;
			}
		}
		// 记录TagSizeFamily信息；
		CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'#',CurrLogBuffSize);
		CurrLogBuffSize = IntAdd2StringSimple(LogBuffContent,FRNum,CurrLogBuffSize);
		CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'\t',CurrLogBuffSize);
		CurrLogBuffSize = IntAdd2StringSimple(LogBuffContent,RFNum,CurrLogBuffSize);
		CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'\n',CurrLogBuffSize);
		
		// 初始化均为Dup；
		if(!BamGenJumpFlag)
		{
			memset(DupFlag,1,PairNum * sizeof(unsigned char));
		}
		
		// 假如所有相关reads当成一个UMI整体。
		if(FakeFlag)
		{
			if(!BamGenJumpFlag)
			{
				// 挑选碱基质量和最大的Reads对；
				MaxId = 0;
				MaxBQ = 0;
				for(i = 0;i < PairNum;i ++)
				{
					tBQ = TotalBaseQualityGetFromSam(fid,SortInfo4MapFull,Info4Shift,Read1[i],Read2[i]);
					if(tBQ > MaxBQ)
					{
						MaxId = i;
						MaxBQ = tBQ;
					}
				}
				DupFlag[MaxId] = 0;
			}
		}
		// 逐个UMI进行整理
		else
		{
			UMIFrom = 0;
			while(UMIFrom < PairNum)
			{
				for(UMITo = UMIFrom + 1;UMITo < PairNum;UMITo ++)
				{
					if(! UMISeqCompare(*(UMISeq + UMITo),*(UMISeq + UMIFrom)))
					{
						break;
					}
				}
				
				// 确定FR和RF数量；
				FRNum = 0;
				RFNum = 0;
				for(i = UMIFrom;i < UMITo;i ++)
				{
					if(*(FRFlag + i) & 0x1)
					{
						FRNum += 1;
					}
					else
					{
						RFNum += 1;
					}
				}
				
				// 记录TagSizeFamily信息；
				CurrLogBuffSize = IntAdd2StringSimple(LogBuffContent,FRNum,CurrLogBuffSize);
				CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'\t',CurrLogBuffSize);
				CurrLogBuffSize = IntAdd2StringSimple(LogBuffContent,RFNum,CurrLogBuffSize);
				CurrLogBuffSize = CharAdd2StringSimple(LogBuffContent,'\n',CurrLogBuffSize);
				// 标记重复；
				if(!BamGenJumpFlag)
				{
					// 挑选碱基质量和最大的Reads对；
					MaxId = 0;
					MaxBQ = 0;
					for(i = UMIFrom;i < UMITo;i ++)
					{
						tBQ = TotalBaseQualityGetFromSam(fid,SortInfo4MapFull,Info4Shift,Read1[i],Read2[i]);
						if(tBQ > MaxBQ)
						{
							MaxId = i;
							MaxBQ = tBQ;
						}
					}
					DupFlag[MaxId] = 0;
				}
				
				UMIFrom = UMITo;
			}
		}
		From = To;
		// 将DupFlag信息保存到Info4Index
		if(!BamGenJumpFlag)
		{
			for(i = 0;i < PairNum;i ++)
			{
				for(j = PairFrom[i];j <= PairTo[i];j ++)
				{
					*(Info4Index + *(SortInfo4MapFull + j)) = DupFlag[i];
				}
			}
		}
		
		// 记录处理量;
		if(From > AccumCheckPoint)
		{
			sprintf(cmd,"Have processed %ld read lines",From);
			TimeLog(start,cmd);
			
			CheckNum ++;
			AccumCheckPoint = AccumCheckMulti * CheckNum;
		}
		if(CurrLogBuffSize > MaxLogBuffSize)
		{
			fwrite(LogBuffContent,1,CurrLogBuffSize,fdlp);
			CurrLogBuffSize = 0;
		}
	}
	if(CurrLogBuffSize)
	{
		fwrite(LogBuffContent,1,CurrLogBuffSize,fdlp);
		CurrLogBuffSize = 0;
	}
	fclose(fid);
	fclose(fdlp);
	
	return 1;
}