#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "BasicRelated.h"

// 文件相关 ========================================
// 按行读取（初次调用时需要将LineEnd和BuffSize均置为0）;
// 使用示例“MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd)”；
unsigned char MultiLineCap(FILE *fid,unsigned char *BuffContent,unsigned int MaxBuffSize,unsigned int *BuffSize,unsigned int *LineStart,unsigned int *LineEnd)
{
	unsigned int Item;
	int BackShift;
	
	// 假如初始状态或者读入的数据已处理完，需要重新读入数据；
	if(*LineEnd == *BuffSize)
	{
		*BuffSize = fread(BuffContent,1,MaxBuffSize,fid);
		if(*BuffSize == 0)
		{
			return 0;
		}
		*LineStart = 0;
		*LineEnd = 0;
	}
	else
	{
		*LineStart = *LineEnd + 1;
		*LineEnd = *LineStart;
	}
	
	Item = *LineEnd;
	for(Item;Item < *BuffSize;Item ++)
	{
		if(BuffContent[Item] == '\n')
		{
			*LineEnd = Item;
			return 1;
		}
	}
	
	// relocating;
	BackShift = *LineStart - *BuffSize;
	fseek(fid,BackShift,SEEK_CUR);
	*BuffSize = fread(BuffContent,1,MaxBuffSize,fid);
	if(*BuffSize == 0)
	{
		return 0;
	}
	*LineStart = 0;
	*LineEnd = 0;
	Item = *LineEnd;
	for(Item;Item < *BuffSize;Item ++)
	{
		if(BuffContent[Item] == '\n')
		{
			*LineEnd = Item;
			return 1;
		}
	}
	
	return 0;
}

// 确定文件行数；
unsigned int FileLineCount(unsigned char *File)
{
	FILE *fid;
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int i,LineNum,BuffSize;
	
	LineNum = 0;
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	while(BuffSize = fread(BuffContent,1,MaxBuffSize,fid))
	{
		for(i = 0;i < BuffSize;i ++)
		{
			if(BuffContent[i] == '\n')
			{
				LineNum ++;
			}
		}
	}
	fclose(fid);
	
	return LineNum;
}
unsigned int SamHeadLineCount(unsigned char *File)
{
	FILE *fid;
	unsigned char BuffContent[1000000];
	unsigned int MaxBuffSize = 1000000;
	unsigned int BuffSize = 0;
	unsigned int LineStart = 0;
	unsigned int LineEnd = 0;
	unsigned int i,LineNum;
	
	LineNum = 0;
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
			break;
		}
		LineNum ++;
	}
	fclose(fid);
	
	return LineNum;
}



// 日志 ===========================================
// 用于输出日志记录
unsigned int TimeLog(time_t start,unsigned char *String)
{
	time_t end,dur_min,dur_sec,dur_hour,dur_day;
	char *currt;
	unsigned int i;
	
	time(&end);
	currt = asctime(localtime(&end));
	i = 0;
	while(*(currt + i))
	{
		i ++;
	}
	i = i - 1;
	for(i;i > 0;i --)
	{
		if(*(currt + i) == ' ')
		{
			*(currt + i) = 0;
			break;
		}
	}
	
	dur_sec = end - start;
	if(dur_sec < 60)
	{
		printf("[ %s  %us ] %s.\n",currt,dur_sec,String);
	}
	else
	{
		dur_min = (int)(dur_sec / 60);
		dur_sec = (int)(dur_sec % 60);
		if(dur_min < 60)
		{
			printf("[ %s  %umin%us ] %s.\n",currt,dur_min,dur_sec,String);
		}
		else
		{
			dur_hour = (int)(dur_min / 60);
			dur_min = (int)(dur_min % 60);
			if(dur_hour < 24)
			{
				printf("[ %s  %uh%umin ] %s.\n",currt,dur_hour,dur_min,String);
			}
			else
			{
				dur_day = (int)(dur_hour / 24);
				dur_hour = (int)(dur_hour % 24);
				printf("[ %s  %ud%uh%umin ] %s.\n",currt,dur_day,dur_hour,dur_min,String);
			}
		}
	}
	fflush(stdout);
	
	return 1;
}



// 数字与字符的转换 ===============================
// 用于将字符型的数字转换为纯数字;
unsigned char String2Char(unsigned char *String)
{
	unsigned char i,Multi,Total;
	
	Multi = 10;
	Total = 0;
	i = 0;
	while(String[i])
	{
		Total = Total * Multi + String[i] - 48;
		i ++;
	}
	
	return Total;
}
unsigned int String2Int(unsigned char *String)
{
	unsigned int i,Multi,Total;
	
	Multi = 10;
	Total = 0;
	i = 0;
	while(String[i])
	{
		Total = Total * Multi + String[i] - 48;
		i ++;
	}
	
	return Total;
}
unsigned long String2Long(unsigned char *String)
{
	unsigned long i,Multi,Total;
	
	Multi = 10;
	Total = 0;
	i = 0;
	while(String[i])
	{
		Total = Total * Multi + String[i] - 48;
		i ++;
	}
	
	return Total;
}

// 用于将纯数字转换为字符型的数字;
unsigned int Int2String(unsigned int Num, unsigned char *Char)
{
	unsigned int i,BitNum,tmpId;
	
	BitNum = 10;
	for(i = 1;i < 20;i ++)
	{
		if(Num < BitNum)
		{
			break;
		}
		BitNum = BitNum * 10;
	}
	BitNum = BitNum / 10;
	
	tmpId = 0;
	while(BitNum)
	{
		Char[tmpId] = (unsigned int)(Num / BitNum) % 10 + 48;
		BitNum = (unsigned int)(BitNum / 10);
		tmpId ++;
	}
	Char[tmpId] = '\0';
	
	return tmpId;
}



// 字符串处理相关 ========================================
// 用于后者字符串内容 赋值到 前者中，并返回字符串长度;
unsigned int StringCopy2String(unsigned char *StringTo, unsigned char *StringFrom)
{
	unsigned int i = 0;
	
	while(StringFrom[i])
	{
		StringTo[i] = StringFrom[i];
		i ++;
	}
	StringTo[i] = '\0';
	
	return i;
}

unsigned int IntCopy2String(unsigned char *StringTo, unsigned int Num)
{
	unsigned char StringFrom[5000];
	unsigned int i = 0;
	
	Int2String(Num,StringFrom);
	
	while(StringFrom[i])
	{
		StringTo[i] = StringFrom[i];
		i ++;
	}
	StringTo[i] = '\0';
	
	return i;
}

// 用于将后者字符串内存 连接到 前者字符串末尾，并返回拼接完成后的总长度;
unsigned int StringAdd2String(unsigned char *StringMain, unsigned char *String)
{
	unsigned int i = 0;
	unsigned int j = 0;
	
	while(StringMain[i])
	{
		i ++;
	}
	while(String[j])
	{
		StringMain[i] = String[j];
		i ++;
		j ++;
	}
	StringMain[i] = '\0';
	
	return i;
}

unsigned int CharAdd2String(unsigned char *StringMain, unsigned char Char)
{
	unsigned int i = 0;
	unsigned int j = 0;
	
	while(StringMain[i])
	{
		i ++;
	}
	StringMain[i] = Char;
	i ++;
	StringMain[i] = '\0';
	
	return i;
}
unsigned int CharAdd2StringSimple(unsigned char *StringMain, unsigned char Char, unsigned int Id)
{
	StringMain[Id] = Char;
	Id ++;
	StringMain[Id] = '\0';
	
	return Id;
}

unsigned int IntAdd2String(unsigned char *StringMain, unsigned int Num)
{
	unsigned char String[5000];
	unsigned int i = 0;
	unsigned int j = 0;
	
	Int2String(Num,String);
	
	while(StringMain[i])
	{
		i ++;
	}
	while(String[j])
	{
		StringMain[i] = String[j];
		i ++;
		j ++;
	}
	StringMain[i] = '\0';
	
	return i;
}
unsigned int IntAdd2StringSimple(unsigned char *StringMain, unsigned int Num, unsigned int Id)
{
	unsigned char String[5000];
	unsigned int i = 0;
	
	Int2String(Num,String);
	
	while(String[i])
	{
		StringMain[Id] = String[i];
		i ++;
		Id ++;
	}
	StringMain[Id] = '\0';
	
	return Id;
}

// 用于将后者的dirname部分赋值给前者（包括最后一个“/”），并返回字符串长度;
unsigned int StringDir(unsigned char *StringDir, unsigned char *String)
{
	unsigned int i = 0;
	int DirId = -1;
	
	while(String[i])
	{
		if(String[i] == '/')
		{
			DirId = i;
		}
		i ++;
	}
	
	i = 0;
	if(DirId > 0)
	{
		while(i <= DirId)
		{
			StringDir[i] = String[i];
			i ++;
		}
	}
	StringDir[i] = '\0';
	
	return i;
}

// 用于将后者的basename部分赋值给前者，并返回字符串长度;
unsigned int StringBaseName(unsigned char *BaseName, unsigned char *String)
{
	unsigned int i = 0;
	int DirId = -1;
	
	while(String[i])
	{
		if(String[i] == '/')
		{
			DirId = i;
		}
		i ++;
	}
	DirId += 1;
	
	i = 0;
	while(String[DirId])
	{
		BaseName[i] = String[DirId];
		i ++;
		DirId ++;
	}
	BaseName[i] = '\0';
	
	return i;
}

// 用于替换字符串行首、末尾；
unsigned int StringPrefixReplace(unsigned char *String,unsigned char *CurrPrefix,unsigned char *RePrefix)
{
	unsigned char tString[1000];
	unsigned int tLen,CurrLen,AccumLen,i;
	unsigned int ReplaceFlag = 1;
	
	// 检查需要替换掉的字符串是否存在；
	CurrLen = strlen(CurrPrefix);
	for(i = 0;i < CurrLen;i ++)
	{
		if(String[i] != CurrPrefix[i])
		{
			ReplaceFlag = 0;
			break;
		}
	}
	if(!ReplaceFlag && CurrLen > 0)
	{
		printf("[ Warning ] Prefix Replace failed from %s to %s in %s\n",CurrPrefix,RePrefix,String);
		return ReplaceFlag;
	}
	
	// 组合到信息的字符串；
	tLen = 0;
	AccumLen = 0;
	while(RePrefix[tLen])
	{
		tString[AccumLen] = RePrefix[tLen];
		tLen ++;
		AccumLen ++;
	}
	while(String[CurrLen])
	{
		tString[AccumLen] = String[CurrLen];
		CurrLen ++;
		AccumLen ++;
	}
	tString[AccumLen] = '\0';
	// 字符串拷贝；
	StringCopy2String(String,tString);
	
	return ReplaceFlag;
}
unsigned int StringSuffixReplace(unsigned char *String,unsigned char *CurrSuffix,unsigned char *ReSuffix)
{
	unsigned int OriLen,CurrSuffixLen,i,tId;
	unsigned int ReplaceFlag = 1;
	
	// 检查需要替换掉的字符串是否存在；
	OriLen = strlen(String);
	CurrSuffixLen = strlen(CurrSuffix);
	tId = 0;
	for(i = OriLen - CurrSuffixLen;i < OriLen;i ++)
	{
		if(String[i] != CurrSuffix[tId])
		{
			printf("[ Error ] Could not locate %s in the suffix of %s\n",CurrSuffix,String);
			exit(1);
		}
		tId ++;
	}
	
	// 替换；
	tId = 0;
	OriLen = OriLen - CurrSuffixLen;
	while(ReSuffix[tId])
	{
		String[OriLen] = ReSuffix[tId];
		OriLen ++;
		tId ++;
	}
	String[OriLen] = '\0';
	
	return ReplaceFlag;
}

// 用于判断字符串是否相等;
unsigned int IfStringEqual(unsigned char *StringA,unsigned char *StringB)
{
	unsigned int LenA,LenB,i;
	unsigned int EqualFlag = 1;
	
	LenA = strlen(StringA);
	LenB = strlen(StringB);
	if(LenA != LenB)
	{
		EqualFlag = 0;
	}
	else
	{
		for(i = 0;i < LenA;i ++)
		{
			if(StringA[i] != StringB[i])
			{
				EqualFlag = 0;
				break;
			}
		}
	}
	
	return EqualFlag;
}
unsigned int IfStringBegin(unsigned char *StringMain,unsigned char *StringPrefix)
{
	unsigned int i;
	unsigned int MatchFlag = 1;
	
	i = 0;
	while(StringPrefix[i])
	{
		if(StringMain[i])
		{
			if(StringMain[i] != StringPrefix[i])
			{
				MatchFlag = 0;
				break;
			}
			else
			{
				MatchFlag = i;
			}
		}
		else
		{
			MatchFlag = 0;
			break;
		}
		
		i ++;
	}
	
	return MatchFlag;
}
// 用于判断字符串中是否含有某个字符；
unsigned int IfStringHas(unsigned char *String,unsigned char Char)
{
	unsigned int i;
	unsigned int HasFlag = 0;
	
	i = 0;
	while(String[i])
	{
		if(String[i] == Char)
		{
			HasFlag = 1;
			break;
		}
		i ++;
	}
	
	return HasFlag;
}

// 碱基和数字的转换 ===============
unsigned int Base2Bit(unsigned char Base)
{
	unsigned int Num;
	
	if(Base == 'A')
	{
		Num = 0x00;
	}
	else if(Base == 'T')
	{
		Num = 0x01;
	}
	else if(Base == 'C')
	{
		Num = 0x02;
	}
	else if(Base == 'G')
	{
		Num = 0x03;
	}
	else
	{
		Num = 0x04;
	}
	
	return Num;
}

unsigned char Bit2Base(unsigned char Bit)
{
	unsigned char Base;
	
	if(Bit == 0x00)
	{
		Base = 'A';
	}
	else if(Bit == 0x01)
	{
		Base = 'T';
	}
	else if(Bit == 0x02)
	{
		Base = 'C';
	}
	else if(Bit == 0x03)
	{
		Base = 'G';
	}
	else
	{
		Base = 'N';
	}
	
	return Base;
}


// 染色体号转换成编号 =============
unsigned char Chr2Char(unsigned char *String,unsigned char ReversFlag)
{
	// 默认染色体长度最多为100位;
	static unsigned char ChrItem[500][100];
	static unsigned char ChrId = 0;
	unsigned char MatchFlag = 0;
	unsigned char Char,tId,i;
	
	// ==== ReversFlag为非0时取出对应的字符串 ===
	// 假如需要编号转换成字符串；
	if(ReversFlag)
	{
		if(ReversFlag > ChrId)
		{
			printf("[ Error ] Searching Id exceeding (%u > %u)\n",ReversFlag,ChrId);
			exit(1);
		}
		
		ReversFlag --;
		i = 0;
		while(ChrItem[ReversFlag][i])
		{
			String[i] = ChrItem[ReversFlag][i];
			i ++;
		}
		String[i] = '\0';
		
		return ReversFlag;
	}
	
	if(!String[0])
	{
		return ReversFlag;
	}
	// ==== ReversFlag为0时输出的是数字编号 ===
	// 去除字符串首"chr"标记；
	StringPrefixReplace(String,"chr","");
	// 检查是否有历史记录;
	Char = 0;
	for(i = 0;i < ChrId;i ++)
	{
		if(IfStringEqual(ChrItem[i],String))
		{
			Char = i + 1;
			break;
		}
	}
	
	// 没有历史记录就新增；
	if(!Char)
	{
		tId = 0;
		while(String[tId])
		{
			ChrItem[ChrId][tId] = String[tId];
			tId ++;
		}
		ChrItem[ChrId][tId] = '\0';
		ChrId ++;
		
		Char = ChrId;
	}
	
	return Char;
}