#include <stdio.h>
#include <stdlib.h>
#include "BamRelated.h"
#include "BasicRelated.h"

// 获取sam中前100行的最大reads长度;
unsigned int ReadLenConfirmFromSam(unsigned char *File)
{
	FILE *fid;
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int ReadLen = 0;
	unsigned int i,AccumLineNum,tLen,TabNum;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	AccumLineNum = 0;
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			AccumLineNum ++;
			if(AccumLineNum > 1000)
			{
				break;
			}
			
			TabNum = 0;
			tLen = 0;
			for(i = LineStart;i < LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					TabNum ++;
					
					if(TabNum > 9)
					{
						if(tLen > ReadLen)
						{
							ReadLen = tLen;
						}
						break;
					}
				}
				else if(TabNum == 9)
				{
					tLen ++;
				}
			}
		}
	}
	fclose(fid);
	
	return ReadLen;
}

// 确定reads'ID的格式，比如Illumina、BGI或其它格式;
unsigned int IDFormatConfirmFromSam(unsigned char *File)
{
	FILE *fid;
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	// 默认情况下格式未知;
	unsigned int IDFormatFlag = 0;
	unsigned int i,ReadIdStart,ReadIdEnd,ColonNum;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	// 获取第一行reads'Id;
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			ReadIdStart = LineStart;
			for(i = LineStart;i < LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					ReadIdEnd = i - 1;
					break;
				}
			}
			
			break;
		}
	}
	fclose(fid);
	
	
	// 计算冒号“:”个数，3~7个冒号之间分隔的都是数字（比如：A00679:63:HGVWCDSXX:4:1403:24569:25911）;
	if(IDFormatFlag == 0)
	{
		IDFormatFlag = 1;
		
		ColonNum = 0;
		for(i = ReadIdStart;i <= ReadIdEnd;i ++)
		{
			if(BuffContent[i] == ':')
			{
				ColonNum ++;
			}
			else if(ColonNum >= 3 && ColonNum <= 6)
			{
				// 不是冒号的时候需要检查是否为数字;
				if(BuffContent[i] < 48 || BuffContent[i] > 57)
				{
					IDFormatFlag = 0;
				}
			}
		}
		
		// 至少需要6个冒号；
		if(ColonNum < 6)
		{
			IDFormatFlag = 0;
		}
	}
	// 符合L.C...R....格式的位BGI格式（比如：E100021302L1C003R03404393399）;
	if(IDFormatFlag == 0)
	{
		for(i = ReadIdStart;i <= ReadIdEnd;i ++)
		{
			if(BuffContent[i] == 'L')
			{
				IDFormatFlag = 2;
				if(BuffContent[i + 1] < 48 || BuffContent[i + 1] > 57)
				{
					IDFormatFlag = 0;
				}
				
				if(BuffContent[i + 2] != 'C')
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 3] < 48 || BuffContent[i + 3] > 57)
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 4] < 48 || BuffContent[i + 4] > 57)
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 5] < 48 || BuffContent[i + 5] > 57)
				{
					IDFormatFlag = 0;
				}
				
				if(BuffContent[i + 6] != 'R')
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 7] < 48 || BuffContent[i + 7] > 57)
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 8] < 48 || BuffContent[i + 8] > 57)
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 9] < 48 || BuffContent[i + 9] > 57)
				{
					IDFormatFlag = 0;
				}
				if(BuffContent[i + 10] < 48 || BuffContent[i + 10] > 57)
				{
					IDFormatFlag = 0;
				}
				
				if(IDFormatFlag == 2)
				{
					break;
				}
			}
		}
	}
	
	return IDFormatFlag;
}

// 获得Sam头信息；
unsigned int HeadAquireFromSam(unsigned char *File,unsigned char *HeadString)
{
	FILE *fid;
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int i,AccumNum;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	// 获取第一行reads'Id;
	AccumNum = 0;
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] == '@')
		{
			for(i = LineStart;i <= LineEnd;i ++)
			{
				HeadString[AccumNum] = BuffContent[i];
				AccumNum ++;
			}
			HeadString[AccumNum] = '\0';
		}
		else
		{
			break;
		}
	}
	fclose(fid);
	
	return AccumNum;
}


// 从sam文件，确认UMI长度（只统计ATCGN的数量）;
unsigned int UmiSizeConfirm(unsigned char *File,unsigned char *UmiPrefix)
{
	FILE *fid;
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int AccumLineNum = 0;
	unsigned int i,UMIFlag,TabNum,tId,UMILen,tUMILen;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	UMILen = 0;
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			AccumLineNum ++;
			if(AccumLineNum > 100)
			{
				break;
			}
			
			TabNum = 0;
			UMIFlag = 0;
			for(i = LineStart;i < LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					TabNum ++;
					if(TabNum > 9)
					{
						UMIFlag = 1;
						
						// 需要列首与指定的字符串标签完全相符;
						tId = 0;
						i ++;
						while(UmiPrefix[tId])
						{
							if(UmiPrefix[tId] != BuffContent[i + tId])
							{
								UMIFlag = 0;
								break;
							}
							tId ++;
						}
						
						if(UMIFlag)
						{
							tUMILen = 0;
							tId += i;
							while(BuffContent[tId] != '\t' && BuffContent[tId] != '\n')
							{
								if(BuffContent[tId] == 'A' || BuffContent[tId] == 'T' || BuffContent[tId] == 'C' || BuffContent[tId] == 'G' || BuffContent[tId] == 'N')
								{
									tUMILen ++;
								}
								tId ++;
							}
						}
					}
				}
				
				if(UMIFlag)
				{
					break;
				}
			}
			
			if(tUMILen > UMILen)
			{
				UMILen = tUMILen;
			}
		}
	}
	fclose(fid);
	
	return UMILen;
}

// Group info for reads re-mapping;
unsigned int RGConfirm(unsigned char *File, unsigned char *RG)
{
	FILE *fid;
	unsigned char RGPrefix[10] = "RG:Z:";
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int AccumLineNum = 0;
	unsigned int MatchFlag = 0;
	unsigned int i,TabNum,tId;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			AccumLineNum ++;
			if(AccumLineNum > 100)
			{
				break;
			}
			
			TabNum = 0;
			for(i = LineStart;i <= LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					TabNum ++;
					
					// 只有第11列开始才可能为RG;
					if(TabNum > 10)
					{
						i ++;
						MatchFlag = 1;
						tId = 0;
						while(RGPrefix[tId])
						{
							if(BuffContent[i + tId] != RGPrefix[tId])
							{
								MatchFlag = 0;
								break;
							}
							tId ++;
						}
						
						if(MatchFlag)
						{
							i = i + tId;
							tId = 0;
							while(BuffContent[i] != '\t' && BuffContent[i] != '\n')
							{
								RG[tId] = BuffContent[i];
								i ++;
								tId ++;
							}
							RG[tId] = '\0';
							break;
						}
					}
				}
			}
			
			if(MatchFlag > 0)
			{
				break;
			}
		}
	}
	fclose(fid);
	
	return MatchFlag;
}

// 获取ReadsID信息（暂时没有用到IDFormatFlag信息）；
unsigned int InfoGet4ReadsID(unsigned char *SamFile,unsigned int IDFormatFlag,unsigned long *Info4IDFull)
{
	FILE *fid;
	unsigned char BuffContent[1000000],ReadsIDString[100];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int ReadsNum = 0;
	unsigned int ColId = 0;
	unsigned int i;
	
	fid = fopen(SamFile,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",SamFile);
		exit(1);
	}
	
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			ColId = 0;
			// 默认统计第11位以后的纯数字；
			for(i = LineStart + 11;i <= LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					break;
				}
				
				if(BuffContent[i] >= 48 && BuffContent[i] <= 57)
				{
					ReadsIDString[ColId] = BuffContent[i];
					ColId ++;
				}
			}
			ReadsIDString[ColId] = '\0';
			
			*(Info4IDFull + ReadsNum) = String2Long(ReadsIDString);
			ReadsNum ++;
		}
	}
	fclose(fid);
	
	return ReadsNum;
}

// 用于根据flag标记和Cigar序列确定插入片段的边界；
unsigned short CigarShiftConfirm(unsigned int Flag, unsigned char *Cigar)
{
	unsigned char CigarFlag;
	unsigned int i,From,To,Multi,Shift,MatchFlag;
	// 用于记录匹配坐标和插入片段边界的距离；
	unsigned short ShiftValue = 0;
	
	if(Cigar[0])
	{
		// 反向互补匹配，边界在3’端；
		if(Flag & 0x10)
		{
			From = 0;
			CigarFlag = '\0';
			// 用于记录是否碰到了匹配坐标位置；
			MatchFlag = 0;
			while(Cigar[From])
			{
				// 确定字母标记的位置；
				To = From;
				while(Cigar[To])
				{
					if(Cigar[To] > 57)
					{
						CigarFlag = Cigar[To];
						break;
					}
					To ++;
				}
				
				// 第一次碰到的M即为匹配坐标；
				if(CigarFlag == 'M')
				{
					MatchFlag = 1;
				}
				// 匹配坐标往后都得计算距离；
				if(MatchFlag)
				{
					// 'M' for match/mismatch, 'D' for deletion, 'N' for skipped region, 'S' for soft clip, 'H' for hard clip;
					if(CigarFlag == 'M' || CigarFlag == 'D' || CigarFlag == 'N' || CigarFlag == 'S' || CigarFlag == 'H')
					{
						Shift = 0;
						Multi = 10;
						for(i = From;i < To;i ++)
						{
							Shift = Shift * Multi + Cigar[i] - 48;
						}
						ShiftValue += Shift & 0xffff;
					}
				}
				
				From = To + 1;
			}
			
			if(ShiftValue)
			{
				ShiftValue = ShiftValue - 1;
			}
		}
		// 正向匹配，边界在5’端；
		else
		{
			i = 0;
			MatchFlag = 0;
			while(Cigar[i])
			{
				if(Cigar[i] > 57)
				{
					if(Cigar[i] == 'S' || Cigar[i] == 'H')
					{
						MatchFlag = i;
					}
					break;
				}
				i ++;
			}
			if(MatchFlag)
			{
				Multi = 10;
				for(i = 0;i < MatchFlag;i ++)
				{
					ShiftValue = ShiftValue * Multi + Cigar[i] - 48;
				}
			}
		}
	}
	else
	{
		// 当Cigar值为“*”，也就是输入的为空字符串时。
		ShiftValue = 0x3fff;
	}
	// Reverse or Forward，16bits，第2位;
	ShiftValue |= (Flag & 0x10) << 10;
	// Read1 or Read2，16bits，第1位;
	ShiftValue |= (Flag & 0x40) << 9;
	
	return ShiftValue;
}

// 获取每条reads记录对应的比对信息；
unsigned int InfoGet4Map(unsigned char *SamFile,unsigned short *Info4MapFullCigar,unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos)
{
	FILE *fid;
	unsigned char BuffContent[1000000],Flag[20],Chr[20],Pos[20],Cigar[100],PChr[20],PPos[20];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int ReadsNum = 0;
	unsigned int i,TabNum,tId,tFlag;
	
	fid = fopen(SamFile,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",SamFile);
		exit(1);
	}
	
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			// 第2、3、4、6、7、8列分别是Flag、Chr、Pos、Cigar、PairedChr、PairedPos；
			TabNum = 0;
			for(i = LineStart;i <= LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					TabNum ++;
					tId = 0;
					// 统计完第8列就可以结束了；
					if(TabNum > 7)
					{
						break;
					}
					continue;
				}
				
				if(TabNum == 1)
				{
					Flag[tId] = BuffContent[i];
					tId ++;
					Flag[tId] = '\0';
				}
				else if(TabNum == 2)
				{
					Chr[tId] = BuffContent[i];
					tId ++;
					Chr[tId] = '\0';
				}
				else if(TabNum == 3)
				{
					Pos[tId] = BuffContent[i];
					tId ++;
					Pos[tId] = '\0';
				}
				else if(TabNum == 5)
				{
					Cigar[tId] = BuffContent[i];
					tId ++;
					Cigar[tId] = '\0';
				}
				else if(TabNum == 6)
				{
					PChr[tId] = BuffContent[i];
					tId ++;
					PChr[tId] = '\0';
				}
				else if(TabNum == 7)
				{
					PPos[tId] = BuffContent[i];
					tId ++;
					PPos[tId] = '\0';
				}
			}
			
			// 未匹配的Reads需要处理，chr置空；
			if(IfStringEqual(Chr,"*"))
			{
				StringCopy2String(Chr,"");
			}
			// 配对染色体假如是“*”或者“=”需要单独处理；
			if(IfStringEqual(PChr,"*"))
			{
				StringCopy2String(PChr,"");
			}
			if(IfStringEqual(PChr,"="))
			{
				StringCopy2String(PChr,Chr);
			}
			// Cigar值为“*”时需要置空；
			if(IfStringEqual(Cigar,"*"))
			{
				StringCopy2String(Cigar,"");
			}
			// Cigar值中含有H的话，肯定有对应的一般匹配，需要忽略；
			if(IfStringHas(Cigar,'H'))
			{
				StringCopy2String(Chr,"");
				StringCopy2String(PChr,"");
			}
			
			tFlag = String2Int(Flag);
			*(Info4MapFullCigar + ReadsNum) = CigarShiftConfirm(tFlag,Cigar);
			*(Info4MapFullLeftChr + ReadsNum) = Chr2Char(Chr,0); 
			*(Info4MapFullLeftPos + ReadsNum) = String2Int(Pos);
			*(Info4MapFullRightChr + ReadsNum) = Chr2Char(PChr,0);
			*(Info4MapFullRightPos + ReadsNum) = String2Int(PPos);
			ReadsNum ++;
		}
	}
	fclose(fid);
	
	return ReadsNum;
}

// 逐个处理Reads对，定位原始配对Reads对，并统一标记比对信息；
unsigned int MapInfoUniform(unsigned int MapReadsNum,unsigned int *SortInfo4ReadsID,unsigned int *GroupInfo4ReadsID,unsigned short *Info4MapFullCigar,unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos,unsigned int DebugFlag)
{
	// 同一条reads不应该有太多比对信息，1000足够了；
	unsigned char Chr,PChr,tChr;
	unsigned char LeftChr[1000],RightChr[1000],tLeftChr[1000],tRightChr[1000];
	unsigned int LeftPos[1000],RightPos[1000],PairedId[2];
	unsigned int Pos,PPos,tPos,tPPos;
	unsigned int From,To,IndexA,IndexB;
	unsigned int i,j,tId,PairMatchFlag,OriFlag,ShiftNum;
	
	From = 0;
	while(From < MapReadsNum)
	{
		// 确定同一ReadsID对应的列表;
		for(To = From + 1;To < MapReadsNum;To ++)
		{
			if(*(GroupInfo4ReadsID + *(SortInfo4ReadsID + To)) != *(GroupInfo4ReadsID + *(SortInfo4ReadsID + From)))
			{
				break;
			}
		}
		
		// 定位原始配对Reads；
		tId = 0;
		for(i = From;i < To;i ++)
		{
			LeftChr[tId] = *(Info4MapFullLeftChr + *(SortInfo4ReadsID + i));
			LeftPos[tId] = *(Info4MapFullLeftPos + *(SortInfo4ReadsID + i));
			RightChr[tId] = *(Info4MapFullRightChr + *(SortInfo4ReadsID + i));
			RightPos[tId] = *(Info4MapFullRightPos + *(SortInfo4ReadsID + i));
			
			tId ++;
		}
		PairMatchFlag = 0;
		for(i = 0;i < tId - 1;i ++)
		{
			IndexA = *(Info4MapFullCigar + *(SortInfo4ReadsID + From + i)) & 0x8000;
			for(j = i + 1;j < tId;j ++)
			{
				IndexB = *(Info4MapFullCigar + *(SortInfo4ReadsID + From + j)) & 0x8000;
				// 只有两条Reads分别为Index1和Index2，且匹配坐标、配对坐标是交互相等的，才能判定它们为原始配对的；
				if(IndexA != IndexB)
				{
					if(LeftChr[i] == RightChr[j] && LeftPos[i] == RightPos[j] && RightChr[i] == LeftChr[j] && RightPos[i] == LeftPos[j])
					{
						PairedId[0] = i;
						PairedId[1] = j;
						PairMatchFlag = 1;
						break;
					}
				}
			}
			if(PairMatchFlag)
			{
				break;
			}
		}
		// 必须得定位到原始配对信息;
		if(PairMatchFlag == 0)
		{
			printf("[ Error ] Not enough read pairs found (Total mapping records %u).\n",tId);
			for(i = From;i < To;i ++)
			{
				IndexA = *(Info4MapFullCigar + *(SortInfo4ReadsID + i)) & 0x8000;
				if(IndexA == 0)
				{
					IndexA = 2;
				}
				else
				{
					IndexA = 1;
				}
				tLeftChr[0] = '\0';
				tRightChr[0] = '\0';
				Chr2Char(tLeftChr,LeftChr[i - From]);
				Chr2Char(tRightChr,RightChr[i - From]);
				printf("[ %u ] Read%u chr%s %u - chr%s %u\n",i - From + 1,IndexA,tLeftChr,LeftPos[i - From],tRightChr,RightPos[i - From]);
			}
			exit(1);
		}
		
		
		// ==== For Debug Only ====
		if(DebugFlag)
		{
			printf("\n");
			for(i = From;i < To;i ++)
			{
				IndexA = *(Info4MapFullCigar + *(SortInfo4ReadsID + i)) & 0x8000;
				if(IndexA == 0)
				{
					IndexA = 2;
				}
				else
				{
					IndexA = 1;
				}
				tLeftChr[0] = '\0';
				tRightChr[0] = '\0';
				Chr2Char(tLeftChr,LeftChr[i - From]);
				Chr2Char(tRightChr,RightChr[i - From]);
				printf("[ %u ] Read%u chr%s %u - chr%s %u\n",i - From + 1,IndexA,tLeftChr,LeftPos[i - From],tRightChr,RightPos[i - From]);
			}
		}
		
		
		// 统一配对信息;
		for(i = 0;i < 2;i ++)
		{
			// Read1 or Read2;
			IndexA = *(Info4MapFullCigar + *(SortInfo4ReadsID + From + PairedId[i])) & 0x8000;
			// + or -;
			OriFlag = *(Info4MapFullCigar + *(SortInfo4ReadsID + From + PairedId[i])) & 0x4000;
			// 假如为0x3fff，表明它是unmamp；
			ShiftNum = *(Info4MapFullCigar + *(SortInfo4ReadsID + From + PairedId[i])) & 0x3fff;
			if(ShiftNum == 0x3fff)
			{
				LeftChr[PairedId[i]] = 0;
				LeftPos[PairedId[i]] = 0;
			}
			else
			{
				// 反向互补配对，真实边界在3’端;
				if(OriFlag)
				{
					LeftPos[PairedId[i]] += ShiftNum;
				}
				// 正向配对，真实边界在5’端；
				else
				{
					// 不过有可能比对到比如chr1:1的位置且左侧有SoftClip的，需要小心，否则第32位会置1；
					if(LeftPos[PairedId[i]] > ShiftNum)
					{
						LeftPos[PairedId[i]] -= ShiftNum;
					}
					else
					{
						LeftPos[PairedId[i]] = ShiftNum - LeftPos[PairedId[i]];
					}
				}
			}
			
			// Read1标记在Left，Read2标记在Right，方向标记在第31位(15+16)，第32位用于标记Read1、Read2信息;
			if(IndexA)
			{
				Chr = LeftChr[PairedId[i]];
				Pos = LeftPos[PairedId[i]];
				if(Pos)
				{
					// 未比对上的Read不能记录方向信息；
					Pos |= OriFlag << 16;
				}
			}
			else
			{
				PChr = LeftChr[PairedId[i]];
				PPos = LeftPos[PairedId[i]];
				if(PPos)
				{
					// 未比对上的Read不能记录方向信息；
					PPos |= OriFlag << 16;
				}
			}
		}
		
		
		// ==== For Debug Only ====
		if(DebugFlag)
		{
			tLeftChr[0] = '\0';
			tRightChr[0] = '\0';
			Chr2Char(tLeftChr,Chr);
			Chr2Char(tRightChr,PChr);
			tPos = Pos & 0x3fffffff;
			tPPos = PPos & 0x3fffffff;
			printf("[ Merge ] chr%s %u - chr%s %u\n",tLeftChr,tPos,tRightChr,tPPos);
		}
		
		
		// 统一赋值；
		tId = 0;
		Chr = Chr & 0xff;
		Pos = Pos & 0xffffffff;
		PChr = PChr & 0xff;
		PPos = PPos & 0xffffffff;
		for(i = From;i < To;i ++)
		{
			*(Info4MapFullLeftChr + *(SortInfo4ReadsID + i)) = Chr;
			*(Info4MapFullLeftPos + *(SortInfo4ReadsID + i)) = Pos;
			*(Info4MapFullRightChr + *(SortInfo4ReadsID + i)) = PChr;
			*(Info4MapFullRightPos + *(SortInfo4ReadsID + i)) = PPos;
			
			// 原始Read1和Read2需要单独标记第32位，额外匹配的不需要标记；
			if(i - From == PairedId[0] || i - From == PairedId[1])
			{
				IndexA = *(Info4MapFullCigar + *(SortInfo4ReadsID + i)) & 0x8000;
				if(IndexA)
				{
					*(Info4MapFullLeftPos + *(SortInfo4ReadsID + i)) |= 0x80000000;
				}
				else
				{
					*(Info4MapFullRightPos + *(SortInfo4ReadsID + i)) |= 0x80000000;
				}
			}
		}
		
		// ==== For Debug Only ====
		if(DebugFlag)
		{
			for(i = 0;i < 2;i ++)
			{
				j = From + PairedId[i];
				printf("[ MapInfo ] %u %x - %u %x\n",*(Info4MapFullLeftChr + *(SortInfo4ReadsID + j)),*(Info4MapFullLeftPos + *(SortInfo4ReadsID + j)),*(Info4MapFullRightChr + *(SortInfo4ReadsID + j)),*(Info4MapFullRightPos + *(SortInfo4ReadsID + j)));
			}
		}
		
		From = To;
	}
	
	return 1;
}

// 用于转移Index信息记录；
unsigned int InfoRecord4Index(unsigned int MapReadsNum,unsigned char *Info4Index,unsigned int *SortInfo4ReadsID,unsigned int *GroupInfo4ReadsID,unsigned char *Info4MapFullLeftChr,unsigned int *Info4MapFullLeftPos,unsigned char *Info4MapFullRightChr,unsigned int *Info4MapFullRightPos)
{
	unsigned int ReadId[2];
	unsigned int From,To,i;
	unsigned long FullPos,PFullPos;
	
	memset(Info4Index,0,MapReadsNum * sizeof(unsigned char));
	
	From = 0;
	while(From < MapReadsNum)
	{
		// 确定同一ReadsID对应的列表;
		for(To = From + 1;To < MapReadsNum;To ++)
		{
			if(*(GroupInfo4ReadsID + *(SortInfo4ReadsID + To)) != *(GroupInfo4ReadsID + *(SortInfo4ReadsID + From)))
			{
				break;
			}
		}
		
		for(i = From;i < To;i ++)
		{
			// Read1;
			if(*(Info4MapFullLeftPos + *(SortInfo4ReadsID + i)) & 0x80000000)
			{
				*(Info4Index + *(SortInfo4ReadsID + i)) = 0x01;
				*(Info4MapFullLeftPos + *(SortInfo4ReadsID + i)) &= 0x7fffffff;
				
				FullPos = *(Info4MapFullLeftChr + *(SortInfo4ReadsID + i));
				FullPos = FullPos << 32 | *(Info4MapFullLeftPos + *(SortInfo4ReadsID + i));
				ReadId[0] = *(SortInfo4ReadsID + i);
			}
			// Read2;
			else if(*(Info4MapFullRightPos + *(SortInfo4ReadsID + i)) & 0x80000000)
			{
				*(Info4Index + *(SortInfo4ReadsID + i)) = 0x02;
				*(Info4MapFullRightPos + *(SortInfo4ReadsID + i)) &= 0x7fffffff;
				
				PFullPos = *(Info4MapFullRightChr + *(SortInfo4ReadsID + i));
				PFullPos = PFullPos << 32 | *(Info4MapFullRightPos + *(SortInfo4ReadsID + i));
				ReadId[1] = *(SortInfo4ReadsID + i);
			}
		}
		
		// Left和Right谁大就标记谁，后续用于统计FR+RF；
		if(FullPos > PFullPos)
		{
			*(Info4Index + ReadId[0]) |= 0x10;
		}
		else
		{
			*(Info4Index + ReadId[1]) |= 0x20;
		}
		
		From = To;
	}
	
	return 1;
}

// 用于将标记的比对分组信息，从Reads对拓展多所有单条reads；
unsigned int ExpandMapGroup(unsigned int *GroupInfo4ReadsID,unsigned int *GroupInfo4Map,unsigned int *GroupInfo4MapFull,unsigned int MapReadsNum)
{
	unsigned int i;
	
	for(i = 0;i < MapReadsNum;i ++)
	{
		// 注意Group编码是从1开始的，需要减一。
		*(GroupInfo4MapFull + i) = *(GroupInfo4Map + *(GroupInfo4ReadsID + i) - 1);
	}
	
	return 1;
}

// 用于获得文件中每行的偏移量；
unsigned int InfoGet4Shift(unsigned char *File,unsigned long *Info4Shift)
{
	FILE *fid;
	unsigned char BuffContent[1000000],ReadsIDString[100];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int ReadsNum = 0;
	unsigned int i;
	unsigned long PointerShift = 0;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	while(MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd))
	{
		if(BuffContent[LineStart] != '@')
		{
			*(Info4Shift + ReadsNum) = PointerShift;
			ReadsNum ++;
		}
		
		PointerShift += LineEnd - LineStart + 1;
	}
	fclose(fid);
	
	return ReadsNum;
}

// 用于获得特定行；
unsigned int SpecificLineGet(FILE *fid,unsigned long Shift,unsigned char *Buff)
{
	unsigned int BuffSize,i,LineEnd;
	
	fseek(fid,Shift,SEEK_SET);
	BuffSize = fread(Buff,1,2000,fid);
	if(BuffSize == 0)
	{
		printf("Cannot locate %ld in fid of TotalBaseQualityGetFromSam.\n",Shift);
		exit(1);
	}
	LineEnd = 0;
	for(i = 0;i < BuffSize;i ++)
	{
		if(Buff[i] == '\n')
		{
			LineEnd = i;
			break;
		}
	}
	
	return LineEnd;
}

// 用于分隔行，得到每列的截取数据；
unsigned int ColSplit(unsigned char *Buff,unsigned int LineStart,unsigned int LineEnd,unsigned char Items[][500])
{
	unsigned int i,TabNum,tId;
	
	TabNum = 0;
	tId = 0;
	for(i = LineStart;i < LineEnd;i ++)
	{
		if(Buff[i] == '\t')
		{
			TabNum ++;
			tId = 0;
		}
		else
		{
			Items[TabNum][tId] = Buff[i];
			tId ++;
			Items[TabNum][tId] = '\0';
		}
	}
	TabNum ++;
	
	return TabNum;
}

// 用于获取Reads对对应的碱基之和
unsigned int TotalBaseQualityGetFromSam(FILE *fid,unsigned int *SortInfo4MapFull,unsigned long *Info4Shift,unsigned int Id4Read1,unsigned int Id4Read2)
{
	unsigned char BuffContent[10000],SplitCol[50][500];
	unsigned int LineEnd;
	unsigned int TotalBQ = 0;
	
	LineEnd = SpecificLineGet(fid,*(Info4Shift + *(SortInfo4MapFull + Id4Read1)),BuffContent);
	ColSplit(BuffContent,0,LineEnd,SplitCol);
	if(!IfStringEqual(SplitCol[5],"*"))
	{
		LineEnd = 0;
		while(SplitCol[10][LineEnd])
		{
			TotalBQ += SplitCol[10][LineEnd];
			LineEnd ++;
		}
	}
	
	LineEnd = SpecificLineGet(fid,*(Info4Shift + *(SortInfo4MapFull + Id4Read2)),BuffContent);
	ColSplit(BuffContent,0,LineEnd,SplitCol);
	if(!IfStringEqual(SplitCol[5],"*"))
	{
		LineEnd = 0;
		while(SplitCol[10][LineEnd])
		{
			TotalBQ += SplitCol[10][LineEnd];
			LineEnd ++;
		}
	}
	
	return TotalBQ;
}

// 用于修改碱基位；
unsigned int FlagRevise(unsigned char *InFile,unsigned char *OutFile,unsigned char *DupFlag)
{
	FILE *fid,*fod;
	unsigned char InBuff[1000000],OutBuff[1000000],tFlag[50];
	unsigned int HeadLen,i,tId;
	unsigned int InBuffSize = 0;
	unsigned int OutBuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int MaxInBuffSize = 1000000;
	unsigned int MaxOutBuffSize = 800000;
	unsigned int ReadsNum = 0;
	
	fod = fopen(OutFile,"w");
	if(fod == NULL)
	{
		printf("File creation failed for %s.\n",OutFile);
		exit(1);
	}
	// 输出Header；
	HeadLen = HeadAquireFromSam(InFile,OutBuff);
	fwrite(OutBuff,sizeof(unsigned char),HeadLen,fod);
	
	fid = fopen(InFile,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",InFile);
		exit(1);
	}
	while(MultiLineCap(fid,InBuff,MaxInBuffSize,&InBuffSize,&LineStart,&LineEnd))
	{
		if(InBuff[LineStart] != '@')
		{
			// 需要修改标记位；
			if(DupFlag[ReadsNum])
			{
				for(i = LineStart;i <= LineEnd;i ++)
				{
					OutBuff[OutBuffSize] = InBuff[i];
					OutBuffSize ++;
					
					if(InBuff[i] == '\t')
					{
						break;
					}
				}
				
				// 获取Flag；
				tId = 0;
				for(i = i + 1;i <= LineEnd;i ++)
				{
					if(InBuff[i] == '\t')
					{
						break;
					}
					
					tFlag[tId] = InBuff[i];
					tId ++;
					tFlag[tId] = '\0';
				}
				// 假如不是unmap、secondary，修改Flag位；
				tId = String2Int(tFlag);
				if((tId & 0x0104) == 0)
				{
					tId |= 0x0400;
					Int2String(tId,tFlag);
				}
				tId = 0;
				while(tFlag[tId])
				{
					OutBuff[OutBuffSize] = tFlag[tId];
					OutBuffSize ++;
					tId ++;
				}
				
				// 保存剩余部分；
				for(i;i <= LineEnd;i ++)
				{
					OutBuff[OutBuffSize] = InBuff[i];
					OutBuffSize ++;
				}
			}
			// 不用修改标记位；
			else
			{
				for(i = LineStart;i <= LineEnd;i ++)
				{
					OutBuff[OutBuffSize] = InBuff[i];
					OutBuffSize ++;
				}
			}
			
			if(OutBuffSize > MaxOutBuffSize)
			{
				fwrite(OutBuff,sizeof(unsigned char),OutBuffSize,fod);
				OutBuffSize = 0;
			}
			ReadsNum ++;
		}
	}
	if(OutBuffSize)
	{
		fwrite(OutBuff,sizeof(unsigned char),OutBuffSize,fod);
	}
	
	fclose(fid);
	fclose(fod);
	
	return ReadsNum;
}

unsigned int LineShow(FILE *fid,unsigned int *SortInfo4MapFull,unsigned long *Info4Shift,unsigned int Id,unsigned int GroupId)
{
	unsigned char BuffContent[10000];
	unsigned int LineEnd;
	
	LineEnd = SpecificLineGet(fid,*(Info4Shift + *(SortInfo4MapFull + Id)),BuffContent);
	
	BuffContent[LineEnd + 1] = '\0';
	printf("%u\t",GroupId);
	printf("%s",BuffContent);
	
	return 1;
}

// 获得UMI序列信息；
unsigned long UMISeqGetFromSam(FILE *fid,unsigned int *SortInfo4MapFull,unsigned long *Info4Shift,unsigned int Id4Read,unsigned char *ColPrefixOfUMI,unsigned int HUmiSize)
{
	unsigned char BuffContent[10000],SplitCol[50][500];
	unsigned int LineEnd,ColNum,i,SeqId,tId,ShiftNum,ShiftBit,MatchFlag;
	unsigned long HalfSeq[2];
	unsigned long UMISeq;
	
	LineEnd = SpecificLineGet(fid,*(Info4Shift + *(SortInfo4MapFull + Id4Read)),BuffContent);
	ColNum = ColSplit(BuffContent,0,LineEnd,SplitCol);
	MatchFlag = 0;
	for(i = 10;i < ColNum;i ++)
	{
		if(tId = IfStringBegin(SplitCol[i],ColPrefixOfUMI))
		{
			tId ++;
			SeqId = 0;
			ShiftNum = 0;
			ShiftBit = 0;
			HalfSeq[0] = 0;
			HalfSeq[1] = 0;
			while(SplitCol[i][tId])
			{
				if(ShiftBit >= 30)
				{
					printf("[ Error ] Only suppport 10 bases on half umi at most (%s).\n",BuffContent);
					exit(1);
				}
				
				// 假如是双端，则需要存储入不同的部分；
				if(HUmiSize > 0 && ShiftNum == HUmiSize)
				{
					SeqId ++;
					if(SeqId > 1)
					{
						printf("[ Error ] More than 2 UMI sequences per one read pair (%s).\n",BuffContent);
						exit(1);
					}
					ShiftNum = 0;
					ShiftBit = 0;
				}
				
				if(SplitCol[i][tId] == 'A')
				{
					HalfSeq[SeqId] |= 0x01 << ShiftBit;
				}
				else if(SplitCol[i][tId] == 'T')
				{
					HalfSeq[SeqId] |= 0x02 << ShiftBit;
				}
				else if(SplitCol[i][tId] == 'C')
				{
					HalfSeq[SeqId] |= 0x03 << ShiftBit;
				}
				else if(SplitCol[i][tId] == 'G')
				{
					HalfSeq[SeqId] |= 0x04 << ShiftBit;
				}
				else if(SplitCol[i][tId] == 'N')
				{
					HalfSeq[SeqId] |= 0x00 << ShiftBit;
				}
				// 非ATCGN直接跳过;
				else
				{
					tId ++;
					continue;
				}
				
				tId ++;
				ShiftNum += 1;
				ShiftBit = ShiftNum * 3;
			}
			
			MatchFlag = 1;
			break;
		}
	}
	if(!MatchFlag)
	{
		printf("[ Error ] Can not locate UMI (With prefix %s) in bam line %s\n",ColPrefixOfUMI,BuffContent);
		exit(1);
	}
	
/*
	// 两部分UMI按同一个规则对大小排序，即便是单侧UMI也会有2个成员。
	if(HalfSeq[0] > HalfSeq[1])
	{
		UMISeq = HalfSeq[1];
		// 默认左侧UMI序列大时为F1R2，右侧UMI序列大时为F2R1。当然这里不一定是FR，也可能是FF，RR，只是标记了区别。
		UMISeq = UMISeq << 30 | HalfSeq[0] | 0x8000000000000000;
	}
	else
	{
		UMISeq = HalfSeq[0];
		UMISeq = UMISeq << 30 | HalfSeq[1];
	}
*/
	UMISeq = HalfSeq[0] << 30 | HalfSeq[1];
	
	return UMISeq;
}