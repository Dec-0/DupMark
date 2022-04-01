#include <stdio.h>
#include <stdlib.h>  // for file handle;
#include <getopt.h> // for argument;
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

time_t start,end,dur_min,dur_sec,dur_hour;
FILE *fid,*fod,*fodp,*fodq,*foddcs;
unsigned char Id1 = 0;
unsigned char Id2 = 1;
unsigned char Id3 = 2;
unsigned char InPath[1000],OutPath[1000],UmiName[100],SamPath[1000],OutSamPath[1000],DupLogPath[1000],Fq1Path[1000],Fq2Path[1000],tmpSamPath[1000],DcsPath[1000];
unsigned char SamtoolsBin[1000],Fq2SamScript[1000];
unsigned char BamHead[1000000],BuffContent[1000000],OutBuff[1000000],DcsBuff[1000000],FqBuff[2][1000000];
unsigned int *DupFlag,*ReadLane,*ReadTile,*ReadX,*ReadY,*Query,*Read,*ReadTotalBQ,*Map1,*Map2,*Map3,**UmiSeq,**Index;
unsigned int BamHeadLen,BamReadNum,MaxArraySize,DebugFlag,UmiFlag,FakeFlag,DStrandFlag,GroupInfoFlag,IDFormatFlag,MinReads,ReadExchangeFlag,FilterFlag,CombineFlag,ReadLen,DupFlagId,OutBuffId,DcsBuffId;
unsigned int BuffSize,UmiNum,HUmiNum,HUmiSize,UmiSize,LineStart,LineEnd;
unsigned int MaxBuffSize = 1000000;
unsigned int MaxDupDistr = 800000;
unsigned int MaxOutBuffSize = 800000;
unsigned int DupDistr[1000000],FqBuffId[2];
unsigned long *ReadShift;


// _________________________________________________
//
//                  Sub functions
// _________________________________________________

//*************
// 日志;
int TimeLog(unsigned char *String)
{
	time(&end);
	dur_sec = end - start;
	if(dur_sec < 60)
	{
		printf("[ %ds ] %s.\n",dur_sec,String);
	}
	else
	{
		dur_min = (int)(dur_sec / 60);
		dur_sec = (int)(dur_sec % 60);
		if(dur_min < 60)
		{
			printf("[ %dmin%ds ] %s.\n",dur_min,dur_sec,String);
		}
		else
		{
			dur_hour = (int)(dur_min / 60);
			dur_min = (int)(dur_min % 60);
			printf("[ %dh%dmin ] %s.\n",dur_hour,dur_min,String);
		}
	}
	
	return 1;
}

// 用于将字符型的数字转换为纯数字;
unsigned int Char2Num(unsigned char *String)
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

// 用于将纯数字转换为字符型的数字;
int Num2Char(unsigned int Num, unsigned char *Char)
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
	
	return 1;
}

// 用于后者字符串内容 赋值到 前者中，并返回字符串长度;
unsigned int StringCopy(unsigned char *StringTo, unsigned char *StringFrom)
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

unsigned int StringCopyWithNum(unsigned char *StringTo, unsigned int Num)
{
	unsigned char StringFrom[2000];
	unsigned int i = 0;
	
	Num2Char(Num,StringFrom);
	
	while(StringFrom[i])
	{
		StringTo[i] = StringFrom[i];
		i ++;
	}
	StringTo[i] = '\0';
	
	return i;
}

// 用于将后者字符串内存 连接到 前者字符串末尾，并返回拼接完成后的总长度;
unsigned int StringAdd(unsigned char *StringMain, unsigned char *String)
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

unsigned int StringAddWithNum(unsigned char *StringMain, unsigned int Num)
{
	unsigned char String[2000];
	unsigned int i = 0;
	unsigned int j = 0;
	
	Num2Char(Num,String);
	
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

// 用于将后者的dirname部分赋值给前者，并返回字符串长度;
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

// 命令行参数输入;
int OptGet(int argc, char *argv[])
{
	unsigned char InFile[10] = "infile";
	unsigned char OutFile[10] = "outfile";
	unsigned char Double[10] = "double";
	unsigned char Min[10] = "min";
	unsigned char Fake[10] = "fake";
	unsigned char UmiTag[10] = "umi";
	unsigned char Save[10] = "save";
	unsigned char Merge[10] = "Merge";
	unsigned char Samtools[10] = "S";
	unsigned char Fq2Sam[10] = "Fq2Sam";
	unsigned char Help[5] = "help";
	unsigned char Group[6] = "group";
	unsigned char OptHash[20],BaseName[1000];
	unsigned char Info[2000] = "\n\n\
 DupMark-4.1    For Duplicates Marking and Consensus Making.\n\
 用于bam去重及UMI条件下的reads合并（SSCS、DCS ConsensusMaking）\n\
 和v4.0相比，新增参数“-g”，用于输出reads'ID的详细分组信息\n\
 同时修改了一个bug，非Illumina的reads格式保存中本应存入tY的存入了tX（虽然对BGI格式没有影响，因为只有23位，没有超过24位）\n\
 Last revised date: 2021.6.30\n\
 Contact:zhangdong_xie@foxmail.com\n\n\
 -i/-infile   ( Required ) Bam file prepare to be dup marked.\n\n\
 -o/-outfile  ( Optional ) Dup-marked bam file.\n\
 -u/-umi      ( Optional ) For the tag stamp for UMI (:Z should be included, like RX:Z).\n\
 -d/-double   ( Optional ) For dual UMI only which will make DCS, otherwise only SSCS (only -d, default: single strand UMI mode).\n\
 -m/-min      ( Optional ) The minimal number of reads for SSCS or DCS making (default: 3).\n\
 -f/fake      ( Optional ) Treat it with umi even if there was no umi.\n\
 -s/-save     ( Optional ) If there was no need to filter out the un-paired reads in SSCS or DCS (only -s, default: filtering, only effective when there was UMI).\n\
 -M/-Merge    ( Optional ) If there was need to merge base and quality in SSCS and DCS combination (only -M, default: no change).\n\
 -S           ( Optional ) Directory for samtools (default: samtools).\n\
 -Fq2Sam      ( Optional ) Directory for Fq2SamScript (default:CurrentFolder/Fq2Sam.pl).\n\
 -g/group     ( Optional ) Flag for only statistics of reads' group info.\n\
 -h/-help     ( Optional ) Help info.\n\n\
 [ Careful ] In DCS mode, base will be converted to 'N' if not satisfy the requirement.\n\
 [ Careful ] Default duplciates groupping mode: the UMI of read1 and read2 will exchange for potential grouping.\n\n";
	unsigned int tmp,i,j,k,tmpId;
	int DirId;
	
	// 不能不指定任何参数;
	if(argc == 1)
	{
		printf("[ Error ] No argument specified.%s",Info);
		exit(1);
	}
	
	// 逐一检查所有输入的参数;
	// OptHash用于记录参数Flag;
	memset(OptHash,0,15*sizeof(unsigned char));
	for(i = 1;i < argc;i ++)
	{
		if(argv[i][0] == '-')
		{
			j = 1;
			// 以防参数命令以“--”开头;
			if(argv[i][j] == '-')
			{
				j ++;
			}
			
			// 逐个核实并确定对应的参数（需要不同参数的首字母不一样）;
			if(argv[i][j] == InFile[0])
			{
				if(OptHash[1])
				{
					printf("[ Error ] Argument -i/-infile specifed more than once.\n");
					exit(1);
				}
				
				// match with "infile";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 6] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 6;k ++)
						{
							if(argv[i][k] != InFile[tmp])
							{
								printf("[ Error ] Wrong format for -infile.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -i.%s",Info);
						exit(1);
					}
				}
				
				// must followed with value;
				i ++;
				StringCopy(InPath,argv[i]);
				OptHash[1] = 1;
			}
			else if(argv[i][j] == OutFile[0])
			{
				if(OptHash[2])
				{
					printf("[ Error ] Argument -o/-outfile specifed more than once.\n");
					exit(1);
				}
				
				// match with "outfile";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 7] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 7;k ++)
						{
							if(argv[i][k] != OutFile[tmp])
							{
								printf("[ Error ] Wrong format for -outfile.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -o.%s",Info);
						exit(1);
					}
				}
				
				// must followed with value;
				i ++;
				StringCopy(OutPath,argv[i]);
				OptHash[2] = 1;
			}
			else if(argv[i][j] == UmiTag[0])
			{
				if(OptHash[3])
				{
					printf("[ Error ] Argument -u/-umi specifed more than once.\n");
					exit(1);
				}
				
				// match with "umi";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 3] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 3;k ++)
						{
							if(argv[i][k] != UmiTag[tmp])
							{
								printf("[ Error ] Wrong format for -umi.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -u.%s",Info);
						exit(1);
					}
				}
				
				// must follow with value;
				i ++;
				if(argv[i][0])
				{
					if(argv[i][0] == '-')
					{
						printf("[ Error ] No umi tag specified.%s",Info);
						exit(1);
					}
				}
				else
				{
					printf("[ Error ] No umi tag specified.%s",Info);
					exit(1);
				}
				
				// 会在字符串末尾自动加上“:”;
				StringCopy(UmiName,argv[i]);
				UmiFlag = StringAdd(UmiName,":");
				UmiFlag -= 1;
				OptHash[3] = 1;
			}
			else if(argv[i][j] == Double[0])
			{
				if(OptHash[4])
				{
					printf("[ Error ] Argument -d/-double specifed more than once.\n");
					exit(1);
				}
				
				// match with "double";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 6] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 6;k ++)
						{
							if(argv[i][k] != Double[tmp])
							{
								printf("[ Error ] Wrong format for -double.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -d.%s",Info);
						exit(1);
					}
				}
				
				OptHash[4] = 1;
			}
			else if(argv[i][j] == Save[0])
			{
				if(OptHash[5])
				{
					printf("[ Error ] Argument -s/-save specifed more than once.\n");
					exit(1);
				}
				
				// match with "save";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 4] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 4;k ++)
						{
							if(argv[i][k] != Save[tmp])
							{
								printf("[ Error ] Wrong format for -save.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -s.%s",Info);
						exit(1);
					}
				}
				
				OptHash[5] = 1;
			}
			else if(argv[i][j] == Merge[0])
			{
				if(OptHash[6])
				{
					printf("[ Error ] Argument -m/-merge specifed more than once.\n");
					exit(1);
				}
				
				// match with "Merge";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 5] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 5;k ++)
						{
							if(argv[i][k] != Merge[tmp])
							{
								printf("[ Error ] Wrong format for -Merge.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -M.%s",Info);
						exit(1);
					}
				}
				
				OptHash[6] = 1;
			}
			else if(argv[i][j] == Samtools[0])
			{
				if(OptHash[7])
				{
					printf("[ Error ] Argument -S specifed more than once.\n");
					exit(1);
				}
				
				if(argv[i][j + 1])
				{
					printf("[ Error ] Wrong format for -S.%s",Info);
					exit(1);
				}
				
				i ++;
				StringCopy(SamtoolsBin,argv[i]);
				OptHash[7] = 1;
			}
			else if(argv[i][j] == Fq2Sam[0])
			{
				if(OptHash[8])
				{
					printf("[ Error ] Argument -F/-Fq2Sam specifed more than once.\n");
					exit(1);
				}
				
				// match with "Fq2Sam";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 6] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 6;k ++)
						{
							if(argv[i][k] != Fq2Sam[tmp])
							{
								printf("[ Error ] Wrong format for -Fq2Sam.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -F.%s",Info);
						exit(1);
					}
				}
				
				i ++;
				StringCopy(Fq2SamScript,argv[i]);
				OptHash[8] = 1;
			}
			else if(argv[i][j] == Min[0])
			{
				if(OptHash[9])
				{
					printf("[ Error ] Argument -m/-min specifed more than once.\n");
					exit(1);
				}
				
				// match with "min";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 3] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 3;k ++)
						{
							if(argv[i][k] != Min[tmp])
							{
								printf("[ Error ] Wrong format for -min.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -m.%s",Info);
						exit(1);
					}
				}
				
				// must followed with value;
				i ++;
				MinReads = Char2Num(argv[i]);
				OptHash[9] = 1;
			}
			else if(argv[i][j] == Fake[0])
			{
				if(OptHash[10])
				{
					printf("[ Error ] Argument -f/-fake specifed more than once.\n");
					exit(1);
				}
				
				// match with "Fake";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 4] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 4;k ++)
						{
							if(argv[i][k] != Fake[tmp])
							{
								printf("[ Error ] Wrong format for -fake.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -fake.%s",Info);
						exit(1);
					}
				}
				
				OptHash[10] = 1;
			}
			else if(argv[i][j] == Group[0])
			{
				// match with "help";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 5] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 5;k ++)
						{
							if(argv[i][k] != Group[tmp])
							{
								printf("[ Error ] Wrong format for -group.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -g.%s",Info);
						exit(1);
					}
				}
				
				OptHash[11] = 1;
			}
			else if(argv[i][j] == Help[0])
			{
				// match with "help";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 4] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 4;k ++)
						{
							if(argv[i][k] != Help[tmp])
							{
								printf("[ Error ] Wrong format for -help.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -h.%s",Info);
						exit(1);
					}
				}
				
				printf("%s",Info);
				exit(1);
			}
		}
		else
		{
			printf("[ Error ] Unknown argument: %s.%s",argv[i],Info);
			exit(1);
		}
	}
	
	// 必须要指定输入文件;
	if(OptHash[1] == 0)
	{
		printf("[ Error ] No infile specified.%s",Info);
		exit(1);
	}
	
	// 假如不指定输出文件，则默认输出到输入文件所属的目录下，并修改后缀bam为dedup.bam;
	if(OptHash[2] == 0)
	{
		i = StringCopy(OutPath,InPath);
		i = i - 3;
		OutPath[i] = 'd';
		OutPath[i + 1] = 'e';
		OutPath[i + 2] = 'd';
		OutPath[i + 3] = 'u';
		OutPath[i + 4] = 'p';
		OutPath[i + 5] = '.';
		OutPath[i + 6] = 'b';
		OutPath[i + 7] = 'a';
		OutPath[i + 8] = 'm';
		OutPath[i + 9] = '\0';
	}
	
	// 确定输入bam解压后的sam文件路径;
	StringDir(SamPath,OutPath);
	StringBaseName(BaseName,InPath);
	i = StringAdd(SamPath,BaseName);
	SamPath[i - 3] = 's';
	
	// 需要make consensus时，相关的参数（比如用于重比对的fq1、fq2、临时sam文件）;
	if(OptHash[3] || OptHash[10])
	{
		i = StringCopy(Fq1Path,OutPath);
		i = i - 4;
		Fq1Path[i] = '_';
		Fq1Path[i + 1] = 'R';
		Fq1Path[i + 2] = '1';
		Fq1Path[i + 3] = '.';
		Fq1Path[i + 4] = 'f';
		Fq1Path[i + 5] = 'q';
		Fq1Path[i + 6] = '\0';
		
		i = StringCopy(Fq2Path,OutPath);
		i = i - 4;
		Fq2Path[i] = '_';
		Fq2Path[i + 1] = 'R';
		Fq2Path[i + 2] = '2';
		Fq2Path[i + 3] = '.';
		Fq2Path[i + 4] = 'f';
		Fq2Path[i + 5] = 'q';
		Fq2Path[i + 6] = '\0';
		
		i = StringCopy(tmpSamPath,OutPath);
		i = i - 4;
		tmpSamPath[i] = '.';
		tmpSamPath[i + 1] = 't';
		tmpSamPath[i + 2] = 'm';
		tmpSamPath[i + 3] = 'p';
		tmpSamPath[i + 4] = '.';
		tmpSamPath[i + 5] = 's';
		tmpSamPath[i + 6] = 'a';
		tmpSamPath[i + 7] = 'm';
		tmpSamPath[i + 8] = '\0';
	}
	
	// 确定输出的sam文件路径;
	i = StringCopy(OutSamPath,OutPath);
	OutSamPath[i - 3] = 's';
	
	// 日志文件;
	i = StringCopy(DupLogPath,OutPath);
	DupLogPath[i - 3] = 'd';
	DupLogPath[i - 2] = 'i';
	DupLogPath[i - 1] = 's';
	DupLogPath[i] = 't';
	DupLogPath[i + 1] = 'r';
	DupLogPath[i + 2] = '.';
	DupLogPath[i + 3] = 'x';
	DupLogPath[i + 4] = 'l';
	DupLogPath[i + 5] = 's';
	DupLogPath[i + 6] = '\0';
	
	// 双链或者单链UMI模式（单侧只能SSCS、双侧可以DCS）;
	if(OptHash[4])
	{
		DStrandFlag = 1;
	}
	else
	{
		DStrandFlag = 0;
	}
	
	// 是否需要过滤在consensus making过程中未满足条件的reads;
	if(OptHash[5])
	{
		FilterFlag = 0;
	}
	else
	{
		FilterFlag = 1;
	}
	
	// 是否需要进行序列合并，或者只是简单地标记重复;
	if(OptHash[6])
	{
		CombineFlag = 1;
	}
	else
	{
		CombineFlag = 0;
	}
	
	// samtools路径;
	if(OptHash[7] == 0)
	{
		StringCopy(SamtoolsBin,"samtools");
	}
	
	// fq2sam.pl脚本路径;
	if(OptHash[8] == 0 && CombineFlag)
	{
		StringDir(Fq2SamScript,argv[0]);
		StringAdd(Fq2SamScript,"Fq2Sam.pl");
	}
	
	// 默认的，能够进行SSCS和DCS合并的最少reads数量;
	if(OptHash[9] == 0)
	{
		MinReads = 3;
	}
	
	// 假如需要在不依赖UMI的条件下进行序列合并;
	if(OptHash[10] == 0)
	{
		FakeFlag = 0;
	}
	else
	{
		FakeFlag = 1;
		
		UmiFlag = 4;
		DStrandFlag = 1;
	}
	
	// 假如只统计reads的分组信息；
	if(OptHash[11])
	{
		GroupInfoFlag = 1;
	}
	
	return 1;
}

// 按行读取（初次调用时需要将LineEnd和BuffSize均置为0）;
int LineCap()
{
	int BackShift;
	
	if(LineEnd == BuffSize)
	{
		BuffSize = fread(BuffContent,1,MaxBuffSize,fid);
		if(BuffSize == 0)
		{
			return 0;
		}
		LineStart = 0;
		LineEnd = 0;
	}
	else
	{
		LineStart = LineEnd + 1;
		LineEnd = LineStart;
	}
	
	for(LineEnd;LineEnd < BuffSize;LineEnd ++)
	{
		if(BuffContent[LineEnd] == '\n')
		{
			return 1;
		}
	}
	
	// relocating;
	BackShift = LineStart - BuffSize;
	fseek(fid,BackShift,SEEK_CUR);
	BuffSize = fread(BuffContent,1,MaxBuffSize,fid);
	if(BuffSize == 0)
	{
		return 0;
	}
	LineStart = 0;
	LineEnd = 0;
	for(LineEnd;LineEnd < BuffSize;LineEnd ++)
	{
		if(BuffContent[LineEnd] == '\n')
		{
			return 1;
		}
	}
	
	return 0;
}

// 确定sam文件行数;
int MaxArrayConfirm(unsigned char *File)
{
	unsigned int i;
	
	MaxArraySize = 0;
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
				MaxArraySize ++;
			}
		}
	}
	fclose(fid);
	MaxArraySize = (unsigned int)(MaxArraySize * 1.05);
	
	return 1;
}

// 从sam文件，确认UMI相关信息，比如序列、长度等;
int UmiSizeConfirm(unsigned char *File)
{
	unsigned int i,tmp,tmpId,tmpNum,tmpFlag;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	UmiSize = 0;
	tmpNum = 0;
	LineEnd = 0;
	BuffSize = 0;
	while(LineCap())
	{
		if(BuffContent[LineStart] != '@')
		{
			if(tmpNum > 100)
			{
				break;
			}
			
			tmpFlag = 0;
			tmp = 0;
			for(i = LineStart;i < LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					tmpFlag = 1;
					
					// 需要列首与指定的字符串标签完全相符;
					tmpId = 0;
					i ++;
					while(UmiName[tmpId])
					{
						if(UmiName[tmpId] != BuffContent[i + tmpId])
						{
							tmpFlag = 0;
							break;
						}
						tmpId ++;
					}
					
					if(tmpFlag)
					{
						tmp = 0;
						tmpId += i;
						while(BuffContent[tmpId] != '\t' && BuffContent[tmpId] != '\n')
						{
							tmpId ++;
							if(BuffContent[tmpId] == '-')
							{
								continue;
							}
							tmp ++;
						}
					}
				}
				if(tmpFlag)
				{
					break;
				}
			}
			
			if(tmp > UmiSize)
			{
				UmiSize = tmp;
			}
			
			tmpNum ++;
		}
	}
	fclose(fid);
	
	if(UmiSize == 0)
	{
		printf("No UMI characters, converting to normal dupmark procedure.\n");
		
		DStrandFlag = 0;
		UmiFlag = 0;
		CombineFlag = 0;
	}
	else
	{
		printf("The size of umi string: %d.\n",UmiSize);
	}
	
	if(DStrandFlag)
	{
		if(UmiSize % 2)
		{
			printf("Number of umi is un-even: %d.\n",UmiSize);
			exit(1);
		}
		
		UmiNum = UmiSize;
		HUmiNum = (unsigned int)(UmiSize / 2);
		// 10个字符占用一个长字符;
		tmp = 0;
		if(HUmiNum % 10)
		{
			tmp = 1;
		}
		HUmiSize = (unsigned int)(HUmiNum / 10) + tmp;
		UmiSize = HUmiSize * 2;
	}
	else if(UmiFlag)
	{
		UmiNum = UmiSize;
		
		tmp = 0;
		if(UmiSize % 10)
		{
			tmp = 1;
		}
		UmiSize = (unsigned int)(UmiSize / 10) + tmp;
	}
	
	return 1;
}

// 获取sam中前100行的最大reads长度;
int ReadLenConfirm(unsigned char *File)
{
	unsigned int i,tmpNum,tmpLen,TabNum;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	LineEnd = 0;
	BuffSize = 0;
	tmpNum = 0;
	ReadLen = 0;
	while(LineCap())
	{
		if(BuffContent[LineStart] != '@')
		{
			if(tmpNum > 100)
			{
				break;
			}
			
			TabNum = 0;
			for(i = LineStart;i <= LineEnd;i ++)
			{
				if(BuffContent[i] == '\t')
				{
					TabNum ++;
					
					if(TabNum > 9)
					{
						if(tmpLen > ReadLen)
						{
							ReadLen = tmpLen;
						}
						break;
					}
					else
					{
						tmpLen = 0;
					}
				}
				else if(TabNum == 9)
				{
					tmpLen ++;
				}
			}
			
			tmpNum ++;
		}
	}
	fclose(fid);
	
	return 1;
}

// 确定reads'ID的格式，比如Illumina、BGI或其它格式;
int IDFormatConfirm(unsigned char *File)
{
	unsigned int i,ReadIdStart,ReadIdEnd,ColonNum;
	// 默认情况下格式未知，此时会保存从末尾开始的最多10个数字;
	IDFormatFlag = 0;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	// 获取第一行reads'Id;
	LineEnd = 0;
	BuffSize = 0;
	ReadIdStart = 0;
	ReadIdEnd = 0;
	while(LineCap())
	{
		if(BuffContent[LineStart] != '@')
		{
			ReadIdStart = LineStart;
			for(i = LineStart;i <= LineEnd;i ++)
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
	// 计算冒号“:”个数，3~7个冒号之间分隔的都是数字;
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
		
		if(ColonNum < 6)
		{
			IDFormatFlag = 0;
		}
	}
	// 符合L.C...R....格式的位BGI格式;
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
	fclose(fid);
	
	if(IDFormatFlag == 1)
	{
		TimeLog("Illumina format for reads' ID");
	}
	else if(IDFormatFlag == 2)
	{
		TimeLog("BGI format for reads' ID");
	}
	else
	{
		TimeLog("Unknown format for reads' ID");
	}
	
	return 1;
}

// Group info for reads re-mapping;
int RGConfirm(unsigned char *File, unsigned char *RG)
{
	unsigned char RGName[10] = "RG:Z:";
	unsigned int i,tmpNum,tmpId,TabNum,tmpFlag;
	
	fid = fopen(File,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",File);
		exit(1);
	}
	
	LineEnd = 0;
	BuffSize = 0;
	tmpNum = 0;
	while(LineCap())
	{
		if(BuffContent[LineStart] != '@')
		{
			if(tmpNum > 1)
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
						tmpFlag = 1;
						tmpId = 0;
						while(RGName[tmpId])
						{
							if(BuffContent[i + tmpId] != RGName[tmpId])
							{
								tmpFlag = 0;
								break;
							}
							tmpId ++;
						}
						
						if(tmpFlag)
						{
							i = i + tmpId;
							tmpId = 0;
							while(BuffContent[i] != '\t' && BuffContent[i] != '\n')
							{
								RG[tmpId] = BuffContent[i];
								i ++;
								tmpId ++;
							}
							RG[tmpId] = '\0';
							break;
						}
					}
				}
			}
			
			tmpNum ++;
		}
	}
	fclose(fid);
	
	return 1;
}

// ————————————————————————————————————
//
//   参数输入及初始化;
//
// ————————————————————————————————————
int ArgumentsInit(int argc, char *argv[])
{
	unsigned char cmd[5000];
	
	// 参数输入及初始化;
	UmiFlag = 0;
	DebugFlag = 0;
	ReadExchangeFlag = 1;
	OptGet(argc,argv);
	
	// bam解压为sam (20G bam to 220G sam, consumes 30 mins);
	TimeLog("Transfering bam to sam ..");
	sprintf(cmd,"%s view -h %s > %s",SamtoolsBin,InPath,SamPath);
	system(cmd);
	
	// 确定所需的内存空间;
	MaxArrayConfirm(SamPath);
	ReadLenConfirm(SamPath);
	IDFormatConfirm(SamPath);
	
	// UMI模式下，从sam文件中获取UMI序列信息;
	if(FakeFlag)
	{
		HUmiSize = 1;
		UmiSize = 2;
	}
	else if(UmiFlag)
	{
		UmiSizeConfirm(SamPath);
	}
	
	return 1;
}
// =======================================================================
// =======================================================================



// 初始化存储reads信息的空间;
int MemoryRequire1()
{
	unsigned char i;
	
	// unsigned int，在64位机器上是4个byte，unsigned long是8个byte;
	// 这里共13个int，1个long int，共 60N Bytes，假如是1G条reads的话就是60G;
	if((ReadLane = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( ReadLane %d).\n",MaxArraySize);
		exit(1);
	}
	if((ReadTile = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( ReadTile %d).\n",MaxArraySize);
		exit(1);
	}
	if((ReadX = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( ReadX %d).\n",MaxArraySize);
		exit(1);
	}
	if((ReadY = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( ReadY %d).\n",MaxArraySize);
		exit(1);
	}
	if((ReadTotalBQ = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( ReadTotalBQ %d).\n",MaxArraySize);
		exit(1);
	}
	if((ReadShift = (unsigned long *)malloc(MaxArraySize * sizeof(unsigned long))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( ReadShift %d).\n",MaxArraySize);
		exit(1);
	}
	if((Map1 = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Map1 %d).\n",MaxArraySize);
		exit(1);
	}
	if((Map2 = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Map2 %d).\n",MaxArraySize);
		exit(1);
	}
	if((Map3 = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Map3 %d).\n",MaxArraySize);
		exit(1);
	}
	if(UmiFlag)
	{
		if((UmiSeq = (unsigned int **)malloc(UmiSize * sizeof(unsigned int *))) == NULL)
		{
			printf("[ Error ] Malloc memory unsuccessfully ( UmiSeqFull %d).\n",MaxArraySize);
			exit(1);
		}
		for(i = 0;i < UmiSize;i ++)
		{
			if((UmiSeq[i] = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
			{
				printf("[ Error ] Malloc memory unsuccessfully ( UmiSeq%d %d).\n",i,MaxArraySize);
				exit(1);
			}
		}
	}
	
	if((Index = (unsigned int **)malloc(3 * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( IndexFull %d).\n",MaxArraySize);
		exit(1);
	}
	for(i = 0;i < 3;i ++)
	{
		if((Index[i] = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
		{
			printf("[ Error ] Malloc memory unsuccessfully ( Index%d %d).\n",i,MaxArraySize);
			exit(1);
		}
	}
	
	return 1;
}

int MemoryRequire2()
{
	if((Query = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Query %d).\n",MaxArraySize);
		exit(1);
	}
	if((Read = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Read %d).\n",MaxArraySize);
		exit(1);
	}
	
	return 1;
}

int MemoryRequire3()
{
	if((DupFlag = (unsigned int *)malloc(MaxArraySize * sizeof(unsigned int))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( DupFlag %d).\n",MaxArraySize);
		exit(1);
	}
	
	return 1;
}

int MemoryFree1()
{
	free(ReadLane);
	free(ReadTile);
	free(ReadX);
	free(ReadY);
	
	return 1;
}

int MemoryFree2()
{
	free(ReadTotalBQ);
	free(ReadShift);
	free(Map1);
	free(Map2);
	free(Map3);
	free(Query);
	free(Read);
	free(Index);
	if(UmiFlag)
	{
		free(UmiSeq);
	}
	
	return 1;
}

int MemoryFree3()
{
	free(DupFlag);
	
	return 1;
}

unsigned int Char2Chr(unsigned char *String)
{
	unsigned int Chr,i,Multi;
	
	Chr = 0;
	i = 0;
	while(String[i])
	{
		i ++;
	}
	if(i > 3)
	{
		if(String[3] == 'X')
		{
			Chr = 23;
		}
		else if(String[3] == 'Y')
		{
			Chr = 24;
		}
		else if(String[3] == 'M')
		{
			Chr = 25;
		}
		else
		{
			Multi = 10;
			i = 3;
			while(String[i])
			{
				Chr = Chr * Multi + String[i] - 48;
				i ++;
			}
			// in case like 'chr6_mann_hap4';
			Chr = Chr & 0x7f;
		}
	}
	
	return Chr;
}

unsigned int Char2Bit(unsigned char Char)
{
	unsigned int Num;
	
	if(Char == 'A')
	{
		Num = 0x00;
	}
	else if(Char == 'T')
	{
		Num = 0x01;
	}
	else if(Char == 'C')
	{
		Num = 0x02;
	}
	else if(Char == 'G')
	{
		Num = 0x03;
	}
	else
	{
		Num = 0x04;
	}
	
	return Num;
}

unsigned char Bit2Char(unsigned char Bit)
{
	unsigned char Char;
	
	if(Bit == 0x00)
	{
		Char = 'A';
	}
	else if(Bit == 0x01)
	{
		Char = 'T';
	}
	else if(Bit == 0x02)
	{
		Char = 'C';
	}
	else if(Bit == 0x03)
	{
		Char = 'G';
	}
	else
	{
		Char = 'N';
	}
	
	return Char;
}

unsigned int SoftClipRevise(unsigned int Flag, unsigned char *Cigar)
{
	unsigned char i,From,To,Multi,CigarFlag,MatchFlag;
	unsigned int ShiftValue,Shift;
	
	ShiftValue = 0;
	MatchFlag = 0;
	if(Flag & 0x10)
	{
		// reverse;
		From = 0;
		To = 0;
		CigarFlag = 0;
		while(Cigar[From])
		{
			while(Cigar[To])
			{
				if(Cigar[To] > 57)
				{
					CigarFlag = Cigar[To];
					break;
				}
				To ++;
			}
			
			if(CigarFlag == 'M')
			{
				MatchFlag = 1;
			}
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
					ShiftValue += Shift;
				}
			}
			
			From = To + 1;
			To = From;
		}
		
		if(ShiftValue)
		{
			ShiftValue = ShiftValue - 1;
		}
		ShiftValue |= 0x4000;
	}
	else
	{
		// forward;
		i = 0;
		while(Cigar[i])
		{
			if(Cigar[i] > 57)
			{
				if(Cigar[i] == 'S')
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
	// read1 or 2;
	ShiftValue |= (Flag & 0x40) << 9;
	
	return ShiftValue;
}

// ————————————————————————————————————
//
//   获取reads信息;
//
// ————————————————————————————————————
int ReadInfoCollect(unsigned char *FilePath)
{
	unsigned char tLane[20],tTile[20],tX[20],tY[20],tFlag[20],tChr[20],tPos[20],tCigar[100],tPChr[20],tPPos[20];
	unsigned int i,j,tmp,tmpId,tmpNum,DSNum,tmpFlag,NumAccum;
	unsigned int ColonNum,TabNum;
	unsigned long PointerShift;
	
	MemoryRequire1();
	
	LineEnd = 0;
	BuffSize = 0;
	BamHeadLen = 0;
	BamReadNum = 0;
	PointerShift = 0;
	fid = fopen(FilePath,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",FilePath);
		exit(1);
	}
	while(LineCap())
	{
		if(BuffContent[LineStart] == '@')
		{
			for(i = LineStart;i <= LineEnd;i ++)
			{
				BamHead[BamHeadLen] = BuffContent[i];
				BamHeadLen ++;
			}
		}
		else
		{
			// mark the seperator;
			ColonNum = 0;
			TabNum = 0;
			*(ReadTotalBQ + BamReadNum) = 0;
			for(i = LineStart;i < LineEnd;i ++)
			{
				if(BuffContent[i] == ':' && TabNum == 0)
				{
					ColonNum ++;
					tmpId = 0;
				}
				else if(BuffContent[i] == '\t')
				{
					ColonNum = 10;
					TabNum ++;
					tmpId = 0;
					
					// 除Illumina外其它格式的存储（比如：E100021302L1C003R03404393399）;
					if(TabNum == 1 && IDFormatFlag != 1)
					{
						// 非Illumina格式，直接取8*4=32位数字;
						NumAccum = 0;
						tLane[0] = '\0';
						tTile[0] = '\0';
						tX[0] = '\0';
						tY[0] = '\0';
						for(j = LineStart;j < i;j ++)
						{
							// 只存储纯数字;
							if(BuffContent[j] >= 48 && BuffContent[j] <= 57)
							{
								NumAccum ++;
								// 存储lane、tile、x、y信息;
								if(NumAccum <= 8)
								{
									tLane[NumAccum - 1] = BuffContent[j];
									tLane[NumAccum] = '\0';
								}
								else if(NumAccum <= 16)
								{
									tTile[NumAccum - 9] = BuffContent[j];
									tTile[NumAccum - 8] = '\0';
								}
								else if(NumAccum <= 24)
								{
									tX[NumAccum - 17] = BuffContent[j];
									tX[NumAccum - 16] = '\0';
								}
								else if(NumAccum <= 32)
								{
									tY[NumAccum - 25] = BuffContent[j];
									tY[NumAccum - 24] = '\0';
								}
							}
						}
					}
					// UMI相关信息的存储;
					else if(TabNum > 10)
					{
						if(FakeFlag)
						{
							*(UmiSeq[0] + BamReadNum) = 0x01;
							*(UmiSeq[1] + BamReadNum) = 0x01;
						}
						else if(UmiFlag)
						{
							i ++;
							tmpFlag = 1;
							while(UmiName[tmpId])
							{
								if(BuffContent[i + tmpId] != UmiName[tmpId])
								{
									tmpFlag = 0;
									break;
								}
								tmpId ++;
							}
							
							if(tmpFlag)
							{
								for(j = 0;j < UmiSize;j ++)
								{
									*(UmiSeq[j] + BamReadNum) = 0;
								}
								
								tmp = tmpId + i;
								tmpNum = 0;
								tmpId = 0;
								if(DStrandFlag)
								{
									DSNum = 0;
									while(BuffContent[tmp] != '\t' && BuffContent[tmp] != '\n')
									{
										if(BuffContent[tmp] == '-')
										{
											tmp ++;
											continue;
										}
										tmp ++;
										*(UmiSeq[tmpId] + BamReadNum) |= Char2Bit(BuffContent[tmp]) << tmpNum;
										tmpNum += 3;
										DSNum ++;
										if(tmpNum > 27 || DSNum == HUmiNum)
										{
											tmpNum = 0;
											tmpId ++;
										}
									}
								}
								else
								{
									while(BuffContent[tmp] != '\t' && BuffContent[tmp] != '\n')
									{
										if(BuffContent[tmp] == '-')
										{
											tmp ++;
											continue;
										}
										tmp ++;
										*(UmiSeq[tmpId] + BamReadNum) |= Char2Bit(BuffContent[tmp]) << tmpNum;
										tmpNum += 3;
										if(tmpNum > 27)
										{
											tmpNum = 0;
											tmpId ++;
										}
									}
								}
								
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
				else if(TabNum <= 10)
				{
					// Illumina格式的存储（比如：A00679:63:HGVWCDSXX:4:1403:24569:25911）;
					if(IDFormatFlag == 1)
					{
						// 存储lane、tile、x、y信息;
						if(ColonNum == 3)
						{
							tLane[tmpId] = BuffContent[i];
							tmpId ++;
							tLane[tmpId] = '\0';
						}
						else if(ColonNum == 4)
						{
							tTile[tmpId] = BuffContent[i];
							tmpId ++;
							tTile[tmpId] = '\0';
						}
						else if(ColonNum == 5)
						{
							tX[tmpId] = BuffContent[i];
							tmpId ++;
							tX[tmpId] = '\0';
						}
						else if(ColonNum == 6)
						{
							tY[tmpId] = BuffContent[i];
							tmpId ++;
							tY[tmpId] = '\0';
						}
					}
					
					// 存储flag、chr、pos、cigar信息;
					if(TabNum == 1)
					{
						tFlag[tmpId] = BuffContent[i];
						tmpId ++;
						tFlag[tmpId] = '\0';
					}
					else if(TabNum == 2)
					{
						tChr[tmpId] = BuffContent[i];
						tmpId ++;
						tChr[tmpId] = '\0';
					}
					else if(TabNum == 3)
					{
						tPos[tmpId] = BuffContent[i];
						tmpId ++;
						tPos[tmpId] = '\0';
					}
					else if(TabNum == 5)
					{
						tCigar[tmpId] = BuffContent[i];
						tmpId ++;
						tCigar[tmpId] = '\0';
					}
					else if(TabNum == 6)
					{
						tPChr[tmpId] = BuffContent[i];
						tmpId ++;
						tPChr[tmpId] = '\0';
					}
					else if(TabNum == 7)
					{
						tPPos[tmpId] = BuffContent[i];
						tmpId ++;
						tPPos[tmpId] = '\0';
					}
					else if(TabNum == 10)
					{
						*(ReadTotalBQ + BamReadNum) += BuffContent[i];
					}
				}
			}
			
			*(ReadLane + BamReadNum) = Char2Num(tLane);
			*(ReadTile + BamReadNum) = Char2Num(tTile);
			*(ReadX + BamReadNum) = Char2Num(tX);
			*(ReadY + BamReadNum) = Char2Num(tY);
			*(ReadShift + BamReadNum) = PointerShift;
			*(Map1 + BamReadNum) = Char2Num(tPos);
			tmp = Char2Num(tFlag);
			tmp = SoftClipRevise(tmp,tCigar);
			if(tPChr[0] == '=')
			{
				// Map2: 0x00(chr) 0x0000(read,direct,shift) 0x00(chr); 
				*(Map2 + BamReadNum) = Char2Chr(tChr) << 24 | tmp << 8 | Char2Chr(tChr);
			}
			else
			{
				*(Map2 + BamReadNum) = Char2Chr(tChr) << 24 | tmp << 8 | Char2Chr(tPChr);
			}
			if(tCigar[0] == '*')
			{
				// None cigar flag;
				*(Map2 + BamReadNum) |= 0x80000000;
				*(ReadTotalBQ + BamReadNum) = 0;
			}
			*(Map3 + BamReadNum) = Char2Num(tPPos);
			
			BamReadNum ++;
		}
		
		PointerShift += LineEnd - LineStart + 1;
	}
	BamHead[BamHeadLen] = '\0';
	*(ReadLane + BamReadNum) = '\0';
	*(ReadTile + BamReadNum) = '\0';
	*(ReadX + BamReadNum) = '\0';
	*(ReadY + BamReadNum) = '\0';
	*(ReadShift + BamReadNum) = '\0';
	*(ReadTotalBQ + BamReadNum) = '\0';
	*(Map1 + BamReadNum) = '\0';
	*(Map2 + BamReadNum) = '\0';
	*(Map3 + BamReadNum) = '\0';
	fclose(fid);
	
	if(DebugFlag)
	{
		for(i = 0;i < BamReadNum;i ++)
		{
			printf("[ original input ] %d:%d:%d:%d\t%x:%x:%x\n",*(ReadLane + i),*(ReadTile + i),*(ReadX + i),*(ReadY + i),*(Map1 + i),*(Map2 + i),*(Map3 + i));
		}
		printf("\n");
	}
	
	return 1;
}
// =======================================================================
// =======================================================================



// 比较坐标（Lane, Tile, X, Y）的大小;
int QueryCompar(unsigned int First, unsigned int Second)
{
	if(*(ReadLane + First) > *(ReadLane + Second))
	{
		return 2;
	}
	else if(*(ReadLane + First) < *(ReadLane + Second))
	{
		return 0;
	}
	
	if(*(ReadTile + First) > *(ReadTile + Second))
	{
		return 2;
	}
	else if(*(ReadTile + First) < *(ReadTile + Second))
	{
		return 0;
	}
	
	if(*(ReadX + First) > *(ReadX + Second))
	{
		return 2;
	}
	else if(*(ReadX + First) < *(ReadX + Second))
	{
		return 0;
	}
	
	if(*(ReadY + First) > *(ReadY + Second))
	{
		return 2;
	}
	else if(*(ReadY + First) < *(ReadY + Second))
	{
		return 0;
	}
	
	return 1;
}

// 对比对信息进行排序，使得ab和ba匹配成功;
int MapConvert()
{
	unsigned int tmpMap1,tmpMap2,tmpMap3;
	unsigned long i;
	unsigned long tmpM1,tmpM2;
	
	for(i = 0;i < BamReadNum;i ++)
	{
		tmpM1 = *(Map1 + i);
		tmpM1 = tmpM1 << 8 | *(Map2 + i) >> 24;
		tmpM2 = *(Map2 + i) & 0xff;
		tmpM2 = tmpM2 << 32 | *(Map3 + i);
		
		if(tmpM2 < tmpM1)
		{
			tmpMap1 = *(Map2 + i) << 24 | *(Map3 + i) >> 8;
			tmpMap2 = *(Map3 + i) << 24 | (*(Map2 + i) << 8 & 0xff0000) | (*(Map2 + i) >> 8 & 0xff00) | *(Map1 + i) >> 24;
			tmpMap3 = *(Map1 + i) << 8 | *(Map2 + i) >> 24;
			
			tmpMap1 = tmpMap1 & 0xffffffff;
			tmpMap2 = tmpMap2 & 0xffffffff;
			tmpMap3 = tmpMap3 & 0xffffffff;
			
			*(Map1 + i) = tmpMap1;
			*(Map2 + i) = tmpMap2;
			*(Map3 + i) = tmpMap3;
		}
	}
	
	return 1;
}

// 对同一reads'ID下的对比信息进行统一规范化处理;
int PreDupMark()
{
	unsigned int From,To,i,j,tmpId,readIdA,readIdB,MatchFlag;
	unsigned int Map1Value,Map2Value,Map3Value,TotalBQ,tmpFlag,CigarFlag,OriFlag,ReadFlag,ShiftNum;
	unsigned int LeftChr[10000],LeftPos[10000],RightChr[10000],RightPos[10000],PairedId[2];
	
	TimeLog("Prepare for dup marking");
	From = 0;
	while(From < BamReadNum)
	{
		// pick the paired ones;
		for(To = From + 1;To < BamReadNum;To ++)
		{
			if(*(Query + *(Index[Id1] + To)) != *(Query + *(Index[Id1] + From)))
			{
				break;
			}
		}
		
		tmpId = 0;
		for(i = From;i < To;i ++)
		{
			LeftChr[tmpId] = (*(Map2 + *(Index[Id1] + i)) >> 24) & 0x7f;
			LeftPos[tmpId] = *(Map1 + *(Index[Id1] + i));
			RightChr[tmpId] = *(Map2 + *(Index[Id1] + i)) & 0x7f;
			RightPos[tmpId] = *(Map3 + *(Index[Id1] + i));
			*(Read + *(Index[Id1] + i)) = 0;
			
			tmpId ++;
		}
		
		MatchFlag = 0;
		for(i = 0;i < tmpId - 1;i ++)
		{
			readIdA = *(Map2 + *(Index[Id1] + i + From)) & 0x800000;
			for(j = i + 1;j < tmpId;j ++)
			{
				readIdB = *(Map2 + *(Index[Id1] + j + From)) & 0x800000;
				if(readIdA != readIdB)
				{
					if(LeftChr[i] == RightChr[j] && LeftPos[i] == RightPos[j] && RightChr[i] == LeftChr[j] && RightPos[i] == LeftPos[j])
					{
						PairedId[0] = i;
						PairedId[1] = j;
						MatchFlag = 1;
						break;
					}
				}
			}
			if(MatchFlag)
			{
				break;
			}
		}
		if(MatchFlag == 0)
		{
			printf("[ Error ] No paired reads (total %d).\n",tmpId);
			for(i = From;i < To;i ++)
			{
				printf("%d\t%d\tRead%d\t%d:%d-%d:%d\t\t",i - From,*(Query + *(Index[Id1] + i)),*(Read + *(Index[Id1] + i)),LeftChr[i - From],LeftPos[i - From],RightChr[i - From],RightPos[i - From]);
				printf("%d:%x:%d\n",*(Map1 + *(Index[Id1] + i)),*(Map2 + *(Index[Id1] + i)),*(Map3 + *(Index[Id1] + i)));
			}
			exit(1);
		}
		
		// calculate the 5', FF, 5' and total base-quality;
		Map2Value = 0;
		TotalBQ = 0;
		MatchFlag = 0;
		for(i = 0;i < 2;i ++)
		{
			CigarFlag = *(Map2 + *(Index[Id1] + PairedId[i] + From)) >> 31;
			tmpFlag = (*(Map2 + *(Index[Id1] + PairedId[i] + From)) >> 8) & 0xffff;
			OriFlag = (tmpFlag >> 14) & 0x01;
			ShiftNum = tmpFlag & 0x3fff;
			ReadFlag = tmpFlag & 0x8000;
			
			// real 5' position;
			if(OriFlag)
			{
				// reverse match, 5' at right side;
				LeftPos[PairedId[i]] += ShiftNum;
			}
			else
			{
				if(ShiftNum > LeftPos[PairedId[i]])
				{
					LeftPos[PairedId[i]] = ShiftNum - LeftPos[PairedId[i]] | 0x80000000;
				}
				else
				{
					LeftPos[PairedId[i]] -= ShiftNum;
				}
			}
			
			// read1;
			if(ReadFlag)
			{
				Map1Value = CigarFlag << 31 | LeftChr[PairedId[i]] << 24 | LeftPos[PairedId[i]] >> 8;
				Map2Value |= LeftPos[PairedId[i]] << 24 | OriFlag << 16;
				
				*(Read + *(Index[Id1] + PairedId[i] + From)) = 1;
				MatchFlag += 1;
			}
			else
			{
				Map2Value |= CigarFlag << 7 | LeftChr[PairedId[i]] | OriFlag << 8;
				Map3Value = LeftPos[PairedId[i]];
				
				*(Read + *(Index[Id1] + PairedId[i] + From)) = 2;
				MatchFlag += 2;
			}
			
			TotalBQ += *(ReadTotalBQ + *(Index[Id1] + PairedId[i] + From));
		}
		
		if(MatchFlag != 3)
		{
			printf("[ Error ] No paired reads in map transfering.\n");
			for(i = From;i < To;i ++)
			{
				printf("%d\t%d\t%x:%x:%x\n",*(Query + *(Index[Id1] + i)),*(Read + *(Index[Id1] + i)),*(Map1 + *(Index[Id1] + i)),*(Map2 + *(Index[Id1] + i)),*(Map3 + *(Index[Id1] + i)));
			}
		}
		
		
		Map1Value = Map1Value & 0xffffffff;
		Map2Value = Map2Value & 0xffffffff;
		Map3Value = Map3Value & 0xffffffff;
		for(i = From;i < To;i ++)
		{
			*(Map1 + *(Index[Id1] + i)) = Map1Value;
			*(Map2+ *(Index[Id1] + i)) = Map2Value;
			*(Map3 + *(Index[Id1] + i)) = Map3Value;
			*(ReadTotalBQ + *(Index[Id1] + i)) = TotalBQ;
		}
		
		
		From = To;
	}
	
	if(ReadExchangeFlag)
	{
		MapConvert();
	}
	
	if(DebugFlag)
	{
		for(i = 0;i < BamReadNum;i ++)
		{
			printf("[ After map transfering ]  %d\tRead%d\t%x:%x:%x\n",*(Query + *(Index[Id1] + i)),*(Read + *(Index[Id1] + i)),*(Map1 + *(Index[Id1] + i)),*(Map2 + *(Index[Id1] + i)),*(Map3 + *(Index[Id1] + i)));
		}
		printf("\n");
	}
	
	return 1;
}

// 按排序结果进行重新编号，释放多余存储空间;
int QueryNameChange()
{
	unsigned int i,tmpId,From,To;
	
	MemoryRequire2();
	
	tmpId = 1;
	From = 0;
	while(From < BamReadNum)
	{
		for(To = From + 1;To < BamReadNum;To ++)
		{
			if(QueryCompar(*(Index[Id1] + To),*(Index[Id1] + From)) != 1)
			{
				break;
			}
		}
		
		for(i = From;i < To;i ++)
		{
			*(Query + *(Index[Id1] + i)) = tmpId;
		}
		tmpId ++;
		
		From = To;
	}
	
	// 假如需要统计reads的分组信息，则不能清空;
	if(! GroupInfoFlag)
	{
		MemoryFree1();
	}
	
	if(DebugFlag)
	{
		for(i = 0;i < BamReadNum;i ++)
		{
			printf("[ After query name change ] %d\t%x:%x:%x\n",*(Query + *(Index[Id1] + i)),*(Map1 + *(Index[Id1] + i)),*(Map2 + *(Index[Id1] + i)),*(Map3 + *(Index[Id1] + i)));
		}
		printf("\n");
	}
	
	PreDupMark();
	
	return 1;
}

// ————————————————————————————————————
//
//   对reads'ID排序;
//
// ————————————————————————————————————
int QueryNameSort()
{
	unsigned int i,j,tmp,tmpId;
	unsigned int MaxDupSpan,DulSpan,MinSpan;
	unsigned int LeftBegin,LeftEnd,RightBegin,RightEnd;
	
	for(i = 0;i < BamReadNum;i ++)
	{
		*(Index[Id1] + i) = i;
	}
	
	// Query sort;
	MaxDupSpan = BamReadNum << 1;
	for(DulSpan = 2;DulSpan < MaxDupSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tmpId = 0;
		
		for(i = 0;i < BamReadNum;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin < BamReadNum)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd >= BamReadNum)
				{
					RightEnd = BamReadNum - 1;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id2] + tmpId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id2] + tmpId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					else if(QueryCompar(*(Index[Id1] + LeftBegin),*(Index[Id1] + RightBegin)))
					{
						*(Index[Id2] + tmpId) = *(Index[Id1] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id2] + tmpId) = *(Index[Id1] + LeftBegin);
						LeftBegin ++;
					}
					tmpId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j < BamReadNum;j ++)
				{
					*(Index[Id2] + tmpId) = *(Index[Id1] + j);
					tmpId ++;
				}
			}
		}
		tmp = Id1;
		Id1 = Id2;
		Id2 = tmp;
	}
	
	if(DebugFlag)
	{
		for(i = 0;i < BamReadNum;i ++)
		{
			printf("[ Before query name change ] %d:%d:%d:%d\t%x:%x:%x\n",*(ReadLane + *(Index[Id1] + i)),*(ReadTile + *(Index[Id1] + i)),*(ReadX + *(Index[Id1] + i)),*(ReadY + *(Index[Id1] + i)),*(Map1 + *(Index[Id1] + i)),*(Map2 + *(Index[Id1] + i)),*(Map3 + *(Index[Id1] + i)));
		}
		printf("\n");
	}
	
	QueryNameChange();
	
	return 1;
}
// =======================================================================
// =======================================================================



// 比较比对坐标组;
int MapCompar(unsigned int First, unsigned int Second, unsigned int SortFlag)
{
	unsigned int TotalFlag,tmpA,tmpB;
	unsigned int tmpMap1[2],tmpMap2[2],tmpMap3[2],Flag[2];
	
	// if SortFlag true, then 3' compare, other 5' compare;
	if(SortFlag)
	{
		if(First & 0x80000000)
		{
			First = First & 0x7fffffff;
			tmpMap1[0] = *(Map1 + First);
			tmpMap2[0] = *(Map2 + First);
			tmpMap3[0] = *(Map3 + First);
		}
		else
		{
			tmpMap1[0] = *(Map2 + First) << 24 | *(Map3 + First) >> 8;
			tmpMap2[0] = *(Map3 + First) << 24 | (*(Map2 + First) << 8 & 0xff0000) | (*(Map2 + First) >> 8 & 0xff00) | *(Map1 + First) >> 24;
			tmpMap3[0] = *(Map1 + First) << 8 | *(Map2 + First) >> 24;
			
			tmpMap1[0] = tmpMap1[0] & 0xffffffff;
			tmpMap2[0] = tmpMap2[0] & 0xffffffff;
			tmpMap3[0] = tmpMap3[0] & 0xffffffff;
		}
		
		if(Second & 0x80000000)
		{
			Second = Second & 0x7fffffff;
			tmpMap1[1] = *(Map1 + Second);
			tmpMap2[1] = *(Map2 + Second);
			tmpMap3[1] = *(Map3 + Second);
		}
		else
		{
			tmpMap1[1] = *(Map2 + Second) << 24 | *(Map3 + Second) >> 8;
			tmpMap2[1] = *(Map3 + Second) << 24 | (*(Map2 + Second) << 8 & 0xff0000) | (*(Map2 + Second) >> 8 & 0xff00) | *(Map1 + Second) >> 24;
			tmpMap3[1] = *(Map1 + Second) << 8 | *(Map2 + Second) >> 24;
			
			tmpMap1[1] = tmpMap1[1] & 0xffffffff;
			tmpMap2[1] = tmpMap2[1] & 0xffffffff;
			tmpMap3[1] = tmpMap3[1] & 0xffffffff;
		}
	}
	else
	{
		if(First & 0x80000000)
		{
			First = First & 0x7fffffff;
			tmpMap1[0] = *(Map2 + First) << 24 | *(Map3 + First) >> 8;
			tmpMap2[0] = *(Map3 + First) << 24 | (*(Map2 + First) << 8 & 0xff0000) | (*(Map2 + First) >> 8 & 0xff00) | *(Map1 + First) >> 24;
			tmpMap3[0] = *(Map1 + First) << 8 | *(Map2 + First) >> 24;
			
			tmpMap1[0] = tmpMap1[0] & 0xffffffff;
			tmpMap2[0] = tmpMap2[0] & 0xffffffff;
			tmpMap3[0] = tmpMap3[0] & 0xffffffff;
		}
		else
		{
			tmpMap1[0] = *(Map1 + First);
			tmpMap2[0] = *(Map2 + First);
			tmpMap3[0] = *(Map3 + First);
		}
		
		if(Second & 0x80000000)
		{
			Second = Second & 0x7fffffff;
			tmpMap1[1] = *(Map2 + Second) << 24 | *(Map3 + Second) >> 8;
			tmpMap2[1] = *(Map3 + Second) << 24 | (*(Map2 + Second) << 8 & 0xff0000) | (*(Map2 + Second) >> 8 & 0xff00) | *(Map1 + Second) >> 24;
			tmpMap3[1] = *(Map1 + Second) << 8 | *(Map2 + Second) >> 24;
			
			tmpMap1[1] = tmpMap1[1] & 0xffffffff;
			tmpMap2[1] = tmpMap2[1] & 0xffffffff;
			tmpMap3[1] = tmpMap3[1] & 0xffffffff;
		}
		else
		{
			tmpMap1[1] = *(Map1 + Second);
			tmpMap2[1] = *(Map2 + Second);
			tmpMap3[1] = *(Map3 + Second);
		}
	}
	
	// first chr and pos;
	if(tmpMap1[0] > tmpMap1[1])
	{
		return 2;
	}
	else if(tmpMap1[0] < tmpMap1[1])
	{
		return 0;
	}
	
	tmpA = tmpMap2[0] >> 16;
	tmpB = tmpMap2[1] >> 16;
	if(tmpA > tmpB)
	{
		return 2;
	}
	else if(tmpA < tmpB)
	{
		return 0;
	}
	
	Flag[0] = tmpMap2[0] & 0x80;
	Flag[1] = tmpMap2[1] & 0x80;
	TotalFlag = Flag[0] + Flag[1];
	if(TotalFlag)
	{
		if(Flag[1] == 0)
		{
			// only for non mapping;
			return 3;
		}
		else
		{
			return 1;
		}
	}
	
	// second chr and pos;
	if(tmpMap2[0] > tmpMap2[1])
	{
		return 2;
	}
	else if(tmpMap2[0] < tmpMap2[1])
	{
		return 0;
	}
	
	if(tmpMap3[0] > tmpMap3[1])
	{
		return 2;
	}
	else if(tmpMap3[0] < tmpMap3[1])
	{
		return 0;
	}
	
	return 1;
}

int UmiCompare(unsigned int IdA, unsigned int IdB)
{
	int i,j,tmp,tmpA,tmpB,EqualFlag;
	unsigned int UmiC[4][20];
	
	EqualFlag = 1;
	tmpA = IdA & 0x7fffffff;
	tmpB = IdB & 0x7fffffff;
	
	if(DStrandFlag)
	{
		for(i = 0;i < UmiSize;i ++)
		{
			UmiC[0][i] = *(UmiSeq[i] + tmpA);
			UmiC[2][i] = *(UmiSeq[i] + tmpB);
		}
		
		tmp = 0;
		for(i = HUmiSize;i < UmiSize;i ++)
		{
			UmiC[1][tmp] = *(UmiSeq[i] + tmpA);
			UmiC[3][tmp] = *(UmiSeq[i] + tmpB);
			
			tmp ++;
		}
		for(i = 0;i < HUmiSize;i ++)
		{
			UmiC[1][tmp] = *(UmiSeq[i] + tmpA);
			UmiC[3][tmp] = *(UmiSeq[i] + tmpB);
			
			tmp ++;
		}
		
		for(i = 2;i < 4;i ++)
		{
			EqualFlag = 1;
			for(j = 0;j < UmiSize;j ++)
			{
				if(UmiC[0][j] != UmiC[i][j])
				{
					EqualFlag = 0;
					break;
				}
			}
			
			if(EqualFlag)
			{
				break;
			}
		}
		
		if(EqualFlag == 0)
		{
			tmp = 0;
			
			for(i = 1;i < 4;i ++)
			{
				for(j = 0;j < UmiSize;j ++)
				{
					if(UmiC[i][j] < UmiC[tmp][j])
					{
						tmp = i;
						break;
					}
					else if(UmiC[i][j] > UmiC[tmp][j])
					{
						break;
					}
				}
			}
			
			// the minimal;
			if(tmp > 1)
			{
				EqualFlag = 2;
			}
		}
	}
	else
	{
		for(i = 0;i < UmiSize;i ++)
		{
			if(*(UmiSeq[i] + tmpA) > *(UmiSeq[i] + tmpB))
			{
				EqualFlag = 2;
				break;
			}
			else if(*(UmiSeq[i] + tmpA) < *(UmiSeq[i] + tmpB))
			{
				EqualFlag = 0;
				break;
			}
		}
	}
	
	return EqualFlag;
}

int QuerySort(unsigned int From, unsigned int To)
{
	unsigned char tId2,tId3;
	unsigned int i,j,tmp,tmpId,tmpA,tmpB;
	unsigned int DulSpan,MinSpan,MaxDulSpan,LeftBegin,LeftEnd,RightBegin,RightEnd;
	
	tId2 = Id2;
	tId3 = Id3;
	
	tmp = To - From + 1;
	MaxDulSpan = tmp << 1;
	for(DulSpan = 2;DulSpan < MaxDulSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tmpId = From;
		
		for(i = From;i <= To;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin <= To)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd > To)
				{
					RightEnd = To;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[tId3] + tmpId) = *(Index[tId2] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[tId3] + tmpId) = *(Index[tId2] + LeftBegin);
						LeftBegin ++;
					}
					else
					{
						tmpA = *(Index[tId2] + LeftBegin) & 0x7fffffff;
						tmpB = *(Index[tId2] + RightBegin) & 0x7fffffff;
						if(*(Query + tmpA) > *(Query + tmpB))
						{
							*(Index[tId3] + tmpId) = *(Index[tId2] + RightBegin);
							RightBegin ++;
						}
						else
						{
							*(Index[tId3] + tmpId) = *(Index[tId2] + LeftBegin);
							LeftBegin ++;
						}
					}
					tmpId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j <= To;j ++)
				{
					*(Index[tId3] + tmpId) = *(Index[tId2] + j);
					tmpId ++;
				}
			}
		}
		tmp = tId2;
		tId2 = tId3;
		tId3 = tmp;
	}
	
	if(tId2 != Id2)
	{
		for(i = From;i <= To;i ++)
		{
			*(Index[Id2] + i) = *(Index[tId2] + i);
		}
	}
	
	return 1;
}

int UmiSort(unsigned int From, unsigned int To)
{
	unsigned char tId2,tId3;
	unsigned int i,j,tmp,tmpId;
	unsigned int DulSpan,MinSpan,MaxDulSpan,LeftBegin,LeftEnd,RightBegin,RightEnd;
	
	tId2 = Id2;
	tId3 = Id3;
	
	tmp = To - From + 1;
	MaxDulSpan = tmp << 1;
	for(DulSpan = 2;DulSpan < MaxDulSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tmpId = From;
		
		for(i = From;i <= To;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin <= To)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd > To)
				{
					RightEnd = To;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[tId3] + tmpId) = *(Index[tId2] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[tId3] + tmpId) = *(Index[tId2] + LeftBegin);
						LeftBegin ++;
					}
					else if(UmiCompare(*(Index[tId2] + LeftBegin),*(Index[tId2] + RightBegin)))
					{
						*(Index[tId3] + tmpId) = *(Index[tId2] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[tId3] + tmpId) = *(Index[tId2] + LeftBegin);
						LeftBegin ++;
					}
					tmpId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j <= To;j ++)
				{
					*(Index[tId3] + tmpId) = *(Index[tId2] + j);
					tmpId ++;
				}
			}
		}
		tmp = tId2;
		tId2 = tId3;
		tId3 = tmp;
	}
	
	if(tId2 != Id2)
	{
		for(i = From;i <= To;i ++)
		{
			*(Index[Id2] + i) = *(Index[tId2] + i);
		}
	}
	
	return 1;
}

int DupCount(unsigned int From, unsigned int To)
{
	unsigned int PreQuery,i,tmpDupNum;
	
	PreQuery = *(Query + *(Index[Id2] + From));
	tmpDupNum = 1;
	for(i = From + 1;i <= To;i ++)
	{
		if(*(Query + *(Index[Id2] + i)) != PreQuery)
		{
			PreQuery = *(Query + *(Index[Id2] + i));
			tmpDupNum ++;
		}
	}
	
	if(tmpDupNum >= MaxDupDistr)
	{
		tmpDupNum = MaxDupDistr;
	}
	
	DupDistr[tmpDupNum] ++;
	
	return 1;
}

// 用于标记所有的分组，每次只处理单侧
int DupSort(unsigned int From, unsigned int To, unsigned int SortFlag)
{
	unsigned int i,j,tmp,tmpFlag,tmpId,tmpA,tmpB,tmpC,tmpFrom,tmpTo,tmpLen;
	unsigned int MaxDulSpan,DulSpan,MinSpan;
	unsigned int LeftBegin,LeftEnd,RightBegin,RightEnd;
	
	// Sorting by mapping info;
	tmp = To - From + 1;
	MaxDulSpan = tmp << 1;
	for(DulSpan = 2;DulSpan < MaxDulSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tmpId = From;
		
		for(i = From;i <= To;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin <= To)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd > To)
				{
					RightEnd = To;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + LeftBegin);
						LeftBegin ++;
					}
					else if(MapCompar(*(Index[Id2] + LeftBegin),*(Index[Id2] + RightBegin),SortFlag) > 1)
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + LeftBegin);
						LeftBegin ++;
					}
					tmpId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j <= To;j ++)
				{
					*(Index[Id3] + tmpId) = *(Index[Id2] + j);
					tmpId ++;
				}
			}
		}
		tmp = Id2;
		Id2 = Id3;
		Id3 = tmp;
	}
	
	if(DebugFlag)
	{
		for(i = From;i <= To;i ++)
		{
			tmp = *(Index[Id2] + i);
			if(tmp & 0x80000000)
			{
				// change read1 and read2;
				tmp = tmp & 0x7fffffff;
				if(SortFlag)
				{
					tmpA = *(Map1 + tmp);
					tmpB = *(Map2 + tmp);
					tmpC = *(Map3 + tmp);
				}
				else
				{
					tmpA = *(Map2 + tmp) << 24 | *(Map3 + tmp) >> 8;
					tmpB = *(Map3 + tmp) << 24 | (*(Map2 + tmp) << 8 & 0xff0000) | (*(Map2 + tmp) >> 8 & 0xff00) | *(Map1 + tmp) >> 24;
					tmpC = *(Map1 + tmp) << 8 | *(Map2 + tmp) >> 24;
				}
			}
			else
			{
				if(SortFlag)
				{
					tmpA = *(Map2 + tmp) << 24 | *(Map3 + tmp) >> 8;
					tmpB = *(Map3 + tmp) << 24 | (*(Map2 + tmp) << 8 & 0xff0000) | (*(Map2 + tmp) >> 8 & 0xff00) | *(Map1 + tmp) >> 24;
					tmpC = *(Map1 + tmp) << 8 | *(Map2 + tmp) >> 24;
				}
				else
				{
					tmpA = *(Map1 + tmp);
					tmpB = *(Map2 + tmp);
					tmpC = *(Map3 + tmp);
				}
			}
			
			if(SortFlag)
			{
				printf("[ After dup group, Right compare ]  %d\t%d\t%x:%x:%x\t%d\t",*(Query + tmp),*(Read + tmp),tmpA,tmpB,tmpC,*(ReadTotalBQ + tmp));
			}
			else
			{
				printf("[ After dup group, Left compare ]  %d\t%d\t%x:%x:%x\t%d\t",*(Query + tmp),*(Read + tmp),tmpA,tmpB,tmpC,*(ReadTotalBQ + tmp));
			}
			
			if(UmiFlag)
			{
				for(i = 0;i < UmiSize;i ++)
				{
					printf("%x\t",*(UmiSeq[i] + tmp));
				}
			}
			printf("\n");
		}
		printf("\n");
	}
	
	tmpFrom = From;
	while(tmpFrom <= To)
	{
		for(tmpTo = tmpFrom + 1;tmpTo <= To;tmpTo ++)
		{
			if((MapCompar(*(Index[Id2] + tmpTo),*(Index[Id2] + tmpFrom),SortFlag) & 0x01) == 0)
			{
				break;
			}
		}
		
		if(UmiFlag)
		{
			UmiSort(tmpFrom,tmpTo - 1);
			
			tmpA = tmpFrom;
			while(tmpA < tmpTo)
			{
				for(tmpB = tmpA + 1;tmpB < tmpTo;tmpB ++)
				{
					if(UmiCompare(*(Index[Id2] + tmpB),*(Index[Id2] + tmpA)) != 1)
					{
						break;
					}
				}
				
				tmpFlag = 0;
				for(i = tmpA;i < tmpB;i ++)
				{
					tmp = *(Index[Id2] + i) & 0x7fffffff;
					if(*(DupFlag + tmp))
					{
						tmpId = *(DupFlag + tmp);
						tmpFlag = 1;
						break;
					}
				}
				if(tmpFlag == 0)
				{
					DupFlagId ++;
					tmpId = DupFlagId;
				}
				
				for(i = tmpA;i < tmpB;i ++)
				{
					tmp = *(Index[Id2] + i) & 0x7fffffff;
					*(DupFlag + tmp) = tmpId;
				}
				
				tmpA = tmpB;
			}
		}
		else
		{
			tmpFlag = 0;
			for(i = tmpFrom;i < tmpTo;i ++)
			{
				tmp = *(Index[Id2] + i) & 0x7fffffff;
				// 假如同伴已经有分组ID了，则沿用该ID；
				if(*(DupFlag + tmp))
				{
					tmpId = *(DupFlag + tmp);
					tmpFlag = 1;
					break;
				}
			}
			// 假如同伴没有被赋予任何ID，则顺序编号；
			if(tmpFlag == 0)
			{
				DupFlagId ++;
				tmpId = DupFlagId;
			}
			
			// 对所有相关reads进行编号；
			for(i = tmpFrom;i < tmpTo;i ++)
			{
				tmp = *(Index[Id2] + i) & 0x7fffffff;
				*(DupFlag + tmp) = tmpId;
			}
		}
		
		tmpFrom = tmpTo;
	}
	
	return 1;
}

// 对左右单侧处理后的顺序进行合并排序。
int DupFlagSort()
{
	unsigned int i,j,To,From,tmp,tmpId;
	unsigned int MaxDulSpan,DulSpan,MinSpan;
	unsigned int LeftBegin,LeftEnd,RightBegin,RightEnd;
	
	for(i = 0;i < BamReadNum;i ++)
	{
		*(Index[Id2] + i) = i;
	}
	
	From = 0;
	To = BamReadNum - 1;
	tmp = To - From + 1;
	MaxDulSpan = tmp << 1;
	for(DulSpan = 2;DulSpan < MaxDulSpan;DulSpan = DulSpan << 1)
	{
		MinSpan = DulSpan >> 1;
		tmpId = From;
		
		for(i = From;i <= To;i += DulSpan)
		{
			LeftBegin = i;
			RightBegin = LeftBegin + MinSpan;
			if(RightBegin <= To)
			{
				LeftEnd = RightBegin - 1;
				RightEnd = LeftEnd + MinSpan;
				if(RightEnd > To)
				{
					RightEnd = To;
				}
				
				while(LeftBegin <= LeftEnd || RightBegin <= RightEnd)
				{
					if(LeftBegin > LeftEnd)
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + RightBegin);
						RightBegin ++;
					}
					else if(RightBegin > RightEnd)
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + LeftBegin);
						LeftBegin ++;
					}
					else if(*(DupFlag + *(Index[Id2] + LeftBegin)) > *(DupFlag + *(Index[Id2] + RightBegin)))
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + RightBegin);
						RightBegin ++;
					}
					else
					{
						*(Index[Id3] + tmpId) = *(Index[Id2] + LeftBegin);
						LeftBegin ++;
					}
					tmpId ++;
				}
			}
			else
			{
				for(j = LeftBegin;j <= To;j ++)
				{
					*(Index[Id3] + tmpId) = *(Index[Id2] + j);
					tmpId ++;
				}
			}
		}
		tmp = Id2;
		Id2 = Id3;
		Id3 = tmp;
	}
	
	return 1;
}

// ————————————————————————————————————
//
//   在没有UMI时，不进行序列合并，修改flag位标记后直接输出;
//
// ————————————————————————————————————
int FinalRevise()
{
	unsigned char tmpArray[20];
	unsigned int LineId,tmpValue,tmpId,i;
	
	MemoryFree2();
	fid = fopen(SamPath,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",SamPath);
		exit(1);
	}
	fod = fopen(OutSamPath,"w");
	if(fod == NULL)
	{
		printf("File creation failed for %s.\n",OutSamPath);
		exit(1);
	}
	fwrite(BamHead,1,BamHeadLen,fod);
	LineId = 0;
	OutBuffId = 0;
	LineStart = 0;
	LineEnd = 0;
	BuffSize = 0;
	while(LineCap())
	{
		if(BuffContent[LineStart] != '@')
		{
			if(*(DupFlag + LineId))
			{
				for(i = LineStart;i < LineEnd;i ++)
				{
					OutBuff[OutBuffId] = BuffContent[i];
					OutBuffId ++;
					
					if(BuffContent[i] == '\t')
					{
						break;
					}
				}
				
				
				tmpId = 0;
				for(i = i + 1;i < LineEnd;i ++)
				{
					if(BuffContent[i] == '\t')
					{
						break;
					}
					
					tmpArray[tmpId] = BuffContent[i];
					tmpId ++;
				}
				tmpArray[tmpId] = '\0';
				tmpValue = Char2Num(tmpArray);
				// hard clip 和 cigar位为“*”时，不需要修改;
				if((tmpValue & 0x0104) == 0)
				{
					tmpValue |= 0x0400;
					Num2Char(tmpValue,tmpArray);
				}
				tmpId = 0;
				while(tmpArray[tmpId])
				{
					OutBuff[OutBuffId] = tmpArray[tmpId];
					OutBuffId ++;
					tmpId ++;
				}
				
				for(i;i <= LineEnd;i ++)
				{
					OutBuff[OutBuffId] = BuffContent[i];
					OutBuffId ++;
				}
			}
			else
			{
				for(i = LineStart;i <= LineEnd;i ++)
				{
					OutBuff[OutBuffId] = BuffContent[i];
					OutBuffId ++;
				}
			}
			
			if(OutBuffId > MaxOutBuffSize)
			{
				fwrite(OutBuff,1,OutBuffId,fod);
				OutBuffId = 0;
			}
			
			LineId ++;
		}
	}
	if(OutBuffId)
	{
		fwrite(OutBuff,1,OutBuffId,fod);
	}
	fclose(fid);
	fclose(fod);
	MemoryFree3();
	
	return 1;
}
// =======================================================================
// =======================================================================



unsigned int DupRecord()
{
	unsigned char tmpArray[20];
	unsigned int i,tmpId;
	
	// recording the dups;
	fod = fopen(DupLogPath,"w");
	if(fod == NULL)
	{
		printf("File creation failed for %s.\n",DupLogPath);
		exit(1);
	}
	OutBuffId = 0;
	for(i = 0;i <= MaxDupDistr;i ++)
	{
		if(DupDistr[i])
		{
			Num2Char(i,tmpArray);
			tmpId = 0;
			while(tmpArray[tmpId])
			{
				OutBuff[OutBuffId] = tmpArray[tmpId];
				tmpId ++;
				OutBuffId ++;
			}
			OutBuff[OutBuffId] = '\t';
			OutBuffId ++;
			
			Num2Char(DupDistr[i],tmpArray);
			tmpId = 0;
			while(tmpArray[tmpId])
			{
				OutBuff[OutBuffId] = tmpArray[tmpId];
				tmpId ++;
				OutBuffId ++;
			}
			OutBuff[OutBuffId] = '\n';
			OutBuffId ++;
			
			if(OutBuffId > MaxOutBuffSize)
			{
				fwrite(OutBuff,1,OutBuffId,fod);
				OutBuffId = 0;
			}
		}
	}
	if(OutBuffId)
	{
		fwrite(OutBuff,1,OutBuffId,fod);
	}
	fclose(fod);
	
	return 1;
}

unsigned int IdWithMaxQuality(unsigned int From, unsigned int To)
{
	unsigned int MaxId,MaxQuery,i;
	
	MaxId = From;
	MaxQuery = *(Query + *(Index[Id2] + From));
	for(i = From + 1;i <= To;i ++)
	{
		if(*(ReadTotalBQ + *(Index[Id2] + i)) > *(ReadTotalBQ + *(Index[Id2] + MaxId)))
		{
			MaxId = i;
			MaxQuery = *(Query + *(Index[Id2] + i));
		}
	}
	
	return MaxQuery;
}

int MarkWithNonDup(unsigned int From, unsigned int To, unsigned int QueryId)
{
	unsigned int i;
	
	for(i = From;i <= To;i ++)
	{
		if(*(Query + *(Index[Id2] + i)) == QueryId)
		{
			*(DupFlag + *(Index[Id2] + i)) = 0;
		}
	}
	
	return 1;
}

unsigned long PosGet(unsigned long Shift, unsigned char *Flag, unsigned int Id)
{
	unsigned char tFlag[20],tChr[20],tPos[20],tCigar[100];
	unsigned int i,TabNum,tmpId,tmpPos,tmpFlag,OriFlag,ShiftNum;
	unsigned long FinalPos;
	
	fseek(fid,Shift,SEEK_SET);
	BuffSize = fread(BuffContent,1,2000,fid);
	if(BuffSize == 0)
	{
		printf("Cannot locate %d in sam file.\n",Shift);
		exit(1);
	}
	LineStart = 0;
	for(i = 0;i < BuffSize;i ++)
	{
		if(BuffContent[i] == '\n')
		{
			LineEnd = i;
			break;
		}
	}
	
	TabNum = 0;
	for(i = LineStart;i < LineEnd;i ++)
	{
		if(BuffContent[i] == '\t')
		{
			TabNum ++;
			tmpId = 0;
			
			if(TabNum > 5)
			{
				break;
			}
		}
		else
		{
			if(TabNum == 1)
			{
				tFlag[tmpId] = BuffContent[i];
				tmpId ++;
				tFlag[tmpId] = '\0';
			}
			else if(TabNum == 2)
			{
				tChr[tmpId] = BuffContent[i];
				tmpId ++;
				tChr[tmpId] = '\0';
			}
			else if(TabNum == 3)
			{
				tPos[tmpId] = BuffContent[i];
				tmpId ++;
				tPos[tmpId] = '\0';
			}
			else if(TabNum == 5)
			{
				tCigar[tmpId] = BuffContent[i];
				tmpId ++;
				tCigar[tmpId] = '\0';
			}
		}
	}
	tmpPos = Char2Num(tPos);
	tmpFlag = Char2Num(tFlag);
	tmpFlag = SoftClipRevise(tmpFlag,tCigar);
	OriFlag = (tmpFlag >> 14) & 0x01;
	ShiftNum = tmpFlag & 0x3fff;
	if(OriFlag)
	{
		// reverse match, 5' at right side;
		tmpPos += ShiftNum;
	}
	else
	{
		if(ShiftNum > tmpPos)
		{
			tmpPos = ShiftNum - tmpPos | 0x80000000;
		}
		else
		{
			tmpPos -= ShiftNum;
		}
	}
	FinalPos = Char2Chr(tChr);
	FinalPos = FinalPos << 40 | tmpPos << 8 | OriFlag;
	if(tCigar[0] == '*')
	{
		// None cigar flag;
		FinalPos |= 0x800000000000;
	}
	Flag[Id] = OriFlag;
	
	return FinalPos;
}

int InfoGet(unsigned char BaseArray[][300], unsigned char QualArray[][300], unsigned long Shift, unsigned int Id)
{
	unsigned int i,TabNum,tmpId;
	
	fseek(fid,Shift,SEEK_SET);
	BuffSize = fread(BuffContent,1,2000,fid);
	if(BuffSize == 0)
	{
		printf("Cannot locate %d in sam file.\n",Shift);
		exit(1);
	}
	LineStart = 0;
	for(i = 0;i < BuffSize;i ++)
	{
		if(BuffContent[i] == '\n')
		{
			LineEnd = i;
			break;
		}
	}
	
	TabNum = 0;
	for(i = LineStart;i < LineEnd;i ++)
	{
		if(BuffContent[i] == '\t')
		{
			TabNum ++;
			tmpId = 0;
			
			if(TabNum > 10)
			{
				break;
			}
		}
		else
		{
			if(TabNum == 9)
			{
				BaseArray[Id][tmpId] = BuffContent[i];
				tmpId ++;
			}
			else if(TabNum == 10)
			{
				QualArray[Id][tmpId] = BuffContent[i];
				tmpId ++;
			}
		}
	}
	
	return 1;
}

unsigned int ReadIdGet(unsigned long Shift)
{
	unsigned char tFlag[20];
	unsigned int i,tmpReadId,TabNum,tmpId,tmp;
	
	fseek(fid,Shift,SEEK_SET);
	BuffSize = fread(BuffContent,1,2000,fid);
	if(BuffSize == 0)
	{
		printf("Cannot locate %d in sam file.\n",Shift);
		exit(1);
	}
	LineStart = 0;
	for(i = 0;i < BuffSize;i ++)
	{
		if(BuffContent[i] == '\n')
		{
			LineEnd = i;
			break;
		}
	}
	
	TabNum = 0;
	for(i = LineStart;i < LineEnd;i ++)
	{
		if(BuffContent[i] == '\t')
		{
			TabNum ++;
			tmpId = 0;
			
			if(TabNum > 1)
			{
				break;
			}
		}
		else
		{
			if(TabNum == 1)
			{
				tFlag[tmpId] = BuffContent[i];
				tmpId ++;
				tFlag[tmpId] = '\0';
			}
		}
	}
	tmp = Char2Num(tFlag);
	if(tmp & 0x40)
	{
		tmpReadId = 1;
	}
	else
	{
		tmpReadId = 2;
	}
	
	return tmpReadId;
}

unsigned int BaseQualCombine(unsigned char *Base, unsigned char *Qual, unsigned char *CBase, unsigned char *CQual, unsigned int Items, unsigned int Id)
{
	unsigned char MaxBase,MaxQual;
	unsigned char BBase[6] = "ATCGN";
	unsigned char BakBase[100];
	unsigned int BaseCount[100];
	unsigned int i,MaxBaseCount,ReturnValue;
	
	// the base with most number and the most higher qualtiy;
	memset(BaseCount,0,100*sizeof(unsigned int));
	for(i = 0;i < Items;i ++)
	{
		BaseCount[Base[i]] ++;
	}
	MaxBaseCount = 0;
	for(i = 0;i < 5;i ++)
	{
		if(BaseCount[BBase[i]] > MaxBaseCount)
		{
			MaxBaseCount = BaseCount[BBase[i]];
			MaxBase = BBase[i];
		}
	}
	
	if(MaxBaseCount != Items)
	{
		// if not the same;
		ReturnValue = 1;
	}
	
	if(MaxBaseCount / (double)(Items) >= 0.8)
	{
		// find the most higher quality for the most number base;
		MaxQual = 0;
		for(i = 0;i < Items;i ++)
		{
			if(Base[i] == MaxBase && Qual[i] > MaxQual)
			{
				MaxQual = Qual[i];
			}
		}
		
		CBase[Id] = MaxBase;
		CQual[Id] = MaxQual;
	}
	else
	{
		// locate the most higher quality with the most number base if the dominant number fail the limit;
		MaxQual = 0;
		for(i = 0;i < Items;i ++)
		{
			if(Qual[i] > MaxQual)
			{
				MaxQual = Qual[i];
			}
		}
		
		memset(BakBase,0,100*sizeof(unsigned char));
		for(i = 0;i < Items;i ++)
		{
			if(Qual[i] == MaxQual)
			{
				BakBase[Base[i]] ++;
			}
		}
		
		MaxBaseCount = 0;
		for(i = 0;i < 5;i ++)
		{
			if(BakBase[BBase[i]] && BaseCount[BBase[i]] > MaxBaseCount)
			{
				MaxBaseCount = BaseCount[BBase[i]];
				MaxBase = BBase[i];
			}
		}
		
		// CBase[Id] = MaxBase;
		// CQual[Id] = MaxQual;
		CBase[Id] = 'N';
		CQual[Id] = '#';
	}
	
	ReturnValue = 0;
	if(MaxBase != Base[0] || MaxQual != Qual[0])
	{
		ReturnValue = 1;
	}
	
	return ReturnValue;
}

unsigned char RCConvert(unsigned char Base)
{
	unsigned char RBase;
	
	if(Base == 'A')
	{
		RBase = 'T';
	}
	else if(Base == 'T')
	{
		RBase = 'A';
	}
	else if(Base == 'C')
	{
		RBase = 'G';
	}
	else if(Base == 'G')
	{
		RBase = 'C';
	}
	else
	{
		RBase = 'N';
	}
	
	return RBase;
}

int RevisedPut(unsigned long Shift, unsigned int tReadId, unsigned int Index, unsigned char *Base, unsigned char *Qual, unsigned char RCFlag)
{
	unsigned char tmpChar,TabNum,tmpId,DSNum,tmpNum;
	long i;
	
	fseek(fid,Shift,SEEK_SET);
	BuffSize = fread(BuffContent,1,2000,fid);
	if(BuffSize == 0)
	{
		printf("Cannot locate %d in sam file.\n",Shift);
		exit(1);
	}
	LineStart = 0;
	for(i = 0;i < BuffSize;i ++)
	{
		if(BuffContent[i] == '\n')
		{
			LineEnd = i;
			break;
		}
	}
	
	// readId;
	TabNum = 0;
	FqBuff[tReadId][FqBuffId[tReadId]] = '@';
	FqBuffId[tReadId] ++;
	for(i = LineStart;i <= LineEnd;i ++)
	{
		if(BuffContent[i] == '\t')
		{
			TabNum ++;
			break;
		}
		FqBuff[tReadId][FqBuffId[tReadId]] = BuffContent[i];
		FqBuffId[tReadId] ++;
	}
	
	// UMI sequence;
	if(!FakeFlag)
	{
		FqBuff[tReadId][FqBuffId[tReadId]] = ':';
		FqBuffId[tReadId] ++;
		DSNum = 0;
		tmpNum = 0;
		tmpId = 0;
		if(DStrandFlag)
		{
			while(DSNum < UmiNum)
			{
				tmpChar = *(UmiSeq[tmpId] + Index) >> tmpNum & 0x07;
				FqBuff[tReadId][FqBuffId[tReadId]] = Bit2Char(tmpChar);
				FqBuffId[tReadId] ++;
				
				DSNum ++;
				tmpNum += 3;
				if(tmpNum > 27 || DSNum == HUmiNum)
				{
					tmpNum = 0;
					tmpId ++;
				}
			}
		}
		else
		{
			while(DSNum < UmiNum)
			{
				tmpChar = *(UmiSeq[tmpId] + Index) >> tmpNum & 0x07;
				FqBuff[tReadId][FqBuffId[tReadId]] = Bit2Char(tmpChar);
				FqBuffId[tReadId] ++;
				
				DSNum ++;
				tmpNum += 3;
				if(tmpNum > 27)
				{
					tmpNum = 0;
					tmpId ++;
				}
			}
		}
	}
	FqBuff[tReadId][FqBuffId[tReadId]] = '\t';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = tReadId + 49;
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = ':';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = 'N';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = ':';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = '0';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = '\n';
	FqBuffId[tReadId] ++;
	
	// base and quality;
	if(RCFlag & 0x80)
	{
		RCFlag = RCFlag & 0x7f;
		
		for(i = i + 1;i < LineEnd;i ++)
		{
			if(BuffContent[i] == '\t')
			{
				TabNum ++;
				tmpId = 0;
				if(TabNum > 10)
				{
					break;
				}
			}
			else
			{
				if(TabNum == 9)
				{
					Base[tmpId] = BuffContent[i];
					tmpId ++;
				}
				else if(TabNum == 10)
				{
					Qual[tmpId] = BuffContent[i];
					tmpId ++;
				}
			}
		}
	}
	
	if(RCFlag)
	{
		for(i = ReadLen - 1;i >= 0;i --)
		{
			FqBuff[tReadId][FqBuffId[tReadId]] = RCConvert(Base[i]);
			FqBuffId[tReadId] ++;
		}
	}
	else
	{
		for(i = 0;i < ReadLen;i ++)
		{
			FqBuff[tReadId][FqBuffId[tReadId]] = Base[i];
			FqBuffId[tReadId] ++;
		}
	}
	
	FqBuff[tReadId][FqBuffId[tReadId]] = '\n';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = '+';
	FqBuffId[tReadId] ++;
	FqBuff[tReadId][FqBuffId[tReadId]] = '\n';
	FqBuffId[tReadId] ++;
	if(RCFlag)
	{
		for(i = ReadLen - 1;i >= 0;i --)
		{
			FqBuff[tReadId][FqBuffId[tReadId]] = Qual[i];
			FqBuffId[tReadId] ++;
		}
	}
	else
	{
		for(i = 0;i < ReadLen;i ++)
		{
			FqBuff[tReadId][FqBuffId[tReadId]] = Qual[i];
			FqBuffId[tReadId] ++;
		}
	}
	FqBuff[tReadId][FqBuffId[tReadId]] = '\n';
	FqBuffId[tReadId] ++;
	
	return 1;
}

int OriginalPut(unsigned long Shift, unsigned int Flag)
{
	unsigned char tFlag[20];
	unsigned long i,tmpId,tmpFlag;
	
	fseek(fid,Shift,SEEK_SET);
	BuffSize = fread(BuffContent,1,2000,fid);
	if(BuffSize == 0)
	{
		printf("Cannot locate %d in sam file.\n",Shift);
		exit(1);
	}
	LineStart = 0;
	for(i = 0;i < BuffSize;i ++)
	{
		if(BuffContent[i] == '\n')
		{
			LineEnd = i;
			break;
		}
	}
	
	for(i = LineStart;i <= LineEnd;i ++)
	{
		OutBuff[OutBuffId] = BuffContent[i];
		OutBuffId ++;
		
		if(BuffContent[i] == '\t')
		{
			break;
		}
	}
	
	tmpId = 0;
	for(i = i + 1;i <= LineEnd;i ++)
	{
		if(BuffContent[i] == '\t')
		{
			break;
		}
		
		tFlag[tmpId] = BuffContent[i];
		tmpId ++;
		tFlag[tmpId] = '\0';
	}
	if(Flag)
	{
		tmpFlag = Char2Num(tFlag);
		// flag for unmapping or not primary (hard clip) should not be revised; 
		if((tmpFlag & 0x0104) == 0)
		{
			tmpFlag |= 0x0400;
			Num2Char(tmpFlag,tFlag);
		}
	}
	
	tmpId = 0;
	while(tFlag[tmpId])
	{
		OutBuff[OutBuffId] = tFlag[tmpId];
		OutBuffId ++;
		tmpId ++;
	}
	
	for(i;i <= LineEnd;i ++)
	{
		OutBuff[OutBuffId] = BuffContent[i];
		OutBuffId ++;
	}
	
	return 1;
}

int FqPathRevise()
{
	StringAdd(Fq1Path,".gz");
	StringAdd(Fq2Path,".gz");
	
	return 1;
}

int UmiCheckCompare(unsigned int IdA, unsigned int IdB)
{
	unsigned int i;
	
	for(i = 0;i < UmiSize;i ++)
	{
		if(*(UmiSeq[i] + IdA) != *(UmiSeq[i] + IdB))
		{
			return 0;
		}
	}
	
	return 1;
}

unsigned char UmiCheck(unsigned int From, unsigned int To)
{
	unsigned char RFlag;
	unsigned char tmpArray[20],ABReadsInfo[50000],BAReadsInfo[50000];
	unsigned int i,tmpId,NumA,NumB,tmpNum;
	
	RFlag = 1;
	
	NumA = 0;
	NumB = 0;
	for(i = From;i <= To;i ++)
	{
		if(*(Read + *(Index[Id2] + i)) == 1)
		{
			if(UmiCheckCompare(*(Index[Id2] + i),*(Index[Id2] + From)))
			{
				NumA ++;
				// 看是否需要保存相关readsID信息；
				if(GroupInfoFlag)
				{
					if(NumA == 1)
					{
						StringCopyWithNum(ABReadsInfo,*(ReadLane + *(Index[Id2] + i)));
						StringAdd(ABReadsInfo,":");
						StringAddWithNum(ABReadsInfo,*(ReadTile + *(Index[Id2] + i)));
						StringAdd(ABReadsInfo,":");
						StringAddWithNum(ABReadsInfo,*(ReadX + *(Index[Id2] + i)));
						StringAdd(ABReadsInfo,":");
						StringAddWithNum(ABReadsInfo,*(ReadY + *(Index[Id2] + i)));
					}
					else if(NumA <= 500)
					{
						StringAdd(ABReadsInfo,",");
						StringAddWithNum(ABReadsInfo,*(ReadLane + *(Index[Id2] + i)));
						StringAdd(ABReadsInfo,":");
						StringAddWithNum(ABReadsInfo,*(ReadTile + *(Index[Id2] + i)));
						StringAdd(ABReadsInfo,":");
						StringAddWithNum(ABReadsInfo,*(ReadX + *(Index[Id2] + i)));
						StringAdd(ABReadsInfo,":");
						StringAddWithNum(ABReadsInfo,*(ReadY + *(Index[Id2] + i)));
					}
				}
			}
			else
			{
				NumB ++;
				// 看是否需要保存相关readsID信息；
				if(GroupInfoFlag)
				{
					if(NumB == 1)
					{
						StringCopyWithNum(BAReadsInfo,*(ReadLane + *(Index[Id2] + i)));
						StringAdd(BAReadsInfo,":");
						StringAddWithNum(BAReadsInfo,*(ReadTile + *(Index[Id2] + i)));
						StringAdd(BAReadsInfo,":");
						StringAddWithNum(BAReadsInfo,*(ReadX + *(Index[Id2] + i)));
						StringAdd(BAReadsInfo,":");
						StringAddWithNum(BAReadsInfo,*(ReadY + *(Index[Id2] + i)));
					}
					else if(NumB <= 500)
					{
						StringAdd(BAReadsInfo,",");
						StringAddWithNum(BAReadsInfo,*(ReadLane + *(Index[Id2] + i)));
						StringAdd(BAReadsInfo,":");
						StringAddWithNum(BAReadsInfo,*(ReadTile + *(Index[Id2] + i)));
						StringAdd(BAReadsInfo,":");
						StringAddWithNum(BAReadsInfo,*(ReadX + *(Index[Id2] + i)));
						StringAdd(BAReadsInfo,":");
						StringAddWithNum(BAReadsInfo,*(ReadY + *(Index[Id2] + i)));
					}
				}
			}
		}
	}
	
	if(DStrandFlag)
	{
		//if(NumA < MinReads || NumB < MinReads)
		//{
		//	RFlag = 0;
		//}
		if(NumA + NumB < MinReads)
		{
			RFlag = 0;
		}
		
		Num2Char(NumA,tmpArray);
		tmpId = 0;
		while(tmpArray[tmpId])
		{
			DcsBuff[DcsBuffId] = tmpArray[tmpId];
			DcsBuffId ++;
			tmpId ++;
		}
		DcsBuff[DcsBuffId] = '\t';
		DcsBuffId ++;
		Num2Char(NumB,tmpArray);
		tmpId = 0;
		while(tmpArray[tmpId])
		{
			DcsBuff[DcsBuffId] = tmpArray[tmpId];
			DcsBuffId ++;
			tmpId ++;
		}
		// 输出相关reads信息
		if(GroupInfoFlag)
		{
			if(NumA > 0)
			{
				DcsBuff[DcsBuffId] = '\t';
				DcsBuffId ++;
				tmpId = 0;
				while(ABReadsInfo[tmpId])
				{
					DcsBuff[DcsBuffId] = ABReadsInfo[tmpId];
					DcsBuffId ++;
					tmpId ++;
				}
			}
			if(NumB > 0)
			{
				DcsBuff[DcsBuffId] = '\t';
				DcsBuffId ++;
				tmpId = 0;
				while(BAReadsInfo[tmpId])
				{
					DcsBuff[DcsBuffId] = BAReadsInfo[tmpId];
					DcsBuffId ++;
					tmpId ++;
				}
			}
		}
		DcsBuff[DcsBuffId] = '\n';
		DcsBuffId ++;
	}
	else
	{
		if(NumA < MinReads)
		{
			RFlag = 0;
		}
	}
	
	return RFlag;
}

int DcsPathConfirm()
{
	unsigned int i;
	
	i = StringCopy(DcsPath,OutPath);
	i = i - 3;
	DcsPath[i] = 'd';
	DcsPath[i + 1] = 'c';
	DcsPath[i + 2] = 's';
	DcsPath[i + 3] = '.';
	DcsPath[i + 4] = 'x';
	DcsPath[i + 5] = 'l';
	DcsPath[i + 6] = 's';
	DcsPath[i + 7] = '\0';
	
	return 1;
}

// SSCS、DCS序列合并相关，包括过滤、合并、重比对;
// 注意子函式中数组定义过大容易报段错误，这里从10000改为了1000;
int ReviseAndCombine(unsigned int tFilterFlag, unsigned int tCombineFlag)
{
	// 这里的1000，300指的时重复reads数，以及reads读长;
	unsigned char LeftCBase[300],LeftCQual[300],RightCBase[300],RightCQual[300],TBase[1000],TQual[1000],cmd[5000];
	unsigned char OriFlag[2][1000],LeftBaseArray[1000][300],LeftQualArray[1000][300],RightBaseArray[1000][300],RightQualArray[1000][300];
	unsigned int i,j,tmp,MatchFlag,RevisedFlag;
	unsigned int tmpFrom,tmpTo,tmpA,tmpB,tmpQueryNum,NonDupQueryId,tmpQueryId;
	unsigned int CombineNum[2],ReadId[2][1000];
	unsigned long LeftPos,RightPos;
	unsigned long Pos[2],CoordId[2][1000];
	
	// maximal pairs of reads: 10000;
	// maximal read length: 300;
	
	fid = fopen(SamPath,"r");
	if(fid == NULL)
	{
		printf("File cannot be open: %s\n",SamPath);
		exit(1);
	}
	fod = fopen(OutSamPath,"w");
	if(fod == NULL)
	{
		printf("File creation failed for %s.\n",OutSamPath);
		exit(1);
	}
	fwrite(BamHead,1,BamHeadLen,fod);
	OutBuffId = 0;
	
	// if the reads need to be revised;
	if(tCombineFlag)
	{
		fodp = fopen(Fq1Path,"w");
		if(fodp == NULL)
		{
			printf("File creation failed for %s.\n",Fq1Path);
			exit(1);
		}
		FqBuffId[0] = 0;
		fodq = fopen(Fq2Path,"w");
		if(fodq == NULL)
		{
			printf("File creation failed for %s.\n",Fq2Path);
			exit(1);
		}
		FqBuffId[1] = 0;
	}
	
	// SSCS number for recording;
	if(DStrandFlag)
	{
		DcsPathConfirm();
		foddcs = fopen(DcsPath,"w");
		if(foddcs == NULL)
		{
			printf("File creation failed for %s.\n",DcsPath);
			exit(1);
		}
		DcsBuffId = 0;
	}
	
	tmpFrom = 0;
	while(tmpFrom < BamReadNum)
	{
		// deal with dup group one by another;
		for(tmpTo = tmpFrom + 1;tmpTo < BamReadNum;tmpTo ++)
		{
			if(*(DupFlag + *(Index[Id2] + tmpTo)) != *(DupFlag + *(Index[Id2] + tmpFrom)))
			{
				break;
			}
		}
		
		if(DStrandFlag)
		{
			if(DcsBuffId > MaxOutBuffSize)
			{
				fwrite(DcsBuff,1,DcsBuffId,foddcs);
				DcsBuffId = 0;
			}
		}
		if(UmiCheck(tmpFrom,tmpTo - 1) == 0)
		{
			// minimal 3 reads for SSCS or DCS;
			if(tFilterFlag)
			{
				tmpFrom = tmpTo;
				continue;
			}
		}
		
		// count of Tag family size;
		QuerySort(tmpFrom,tmpTo - 1);
		DupCount(tmpFrom,tmpTo - 1);
		
		// mark the non dup;
		tmpQueryId = IdWithMaxQuality(tmpFrom,tmpTo - 1);
		MarkWithNonDup(tmpFrom,tmpTo - 1,tmpQueryId);
		
		if(tCombineFlag)
		{
			// get the relative Ids of read1 and read2;
			tmpA = tmpFrom;
			tmpQueryNum = 0;
			while(tmpA < tmpTo)
			{
				for(tmpB = tmpA + 1;tmpB < tmpTo;tmpB ++)
				{
					if(*(Query + *(Index[Id2] + tmpB)) != *(Query + *(Index[Id2] + tmpA)))
					{
						break;
					}
				}
				
				// prepare to save the pair of reads with highest quality;
				if(*(Query + *(Index[Id2] + tmpA)) == tmpQueryId)
				{
					NonDupQueryId = tmpQueryNum;
				}
				
				for(i = 1;i < 3;i ++)
				{
					MatchFlag = 0;
					for(j = tmpA;j < tmpB;j ++)
					{
						tmp = *(Index[Id2] + j);
						if(*(Read + tmp) == i)
						{
							ReadId[i - 1][tmpQueryNum] = tmp;
							MatchFlag = 1;
							break;
						}
					}
					if(MatchFlag == 0)
					{
						printf("[ Error ] Read%d missing in SSCS or DCS combination ...\n",i);
						exit(1);
					}
				}
				tmpQueryNum ++;
				
				tmpA = tmpB;
			}
			
			// get the 5' coordinates of read1 and read2;
			for(i = 0;i < tmpQueryNum;i ++)
			{
				LeftPos = PosGet(*(ReadShift + ReadId[0][i]),OriFlag[0],i);
				RightPos = PosGet(*(ReadShift + ReadId[1][i]),OriFlag[1],i);
				
				if(LeftPos & 0x800000000000)
				{
					CoordId[0][i] = 0;
				}
				else
				{
					CoordId[0][i] = LeftPos;
				}
				if(RightPos & 0x800000000000)
				{
					CoordId[1][i] = 0;
				}
				else
				{
					CoordId[1][i] = RightPos;
				}
			}
			Pos[0] = 0;
			for(i = 0;i < tmpQueryNum;i ++)
			{
				if(CoordId[0][i] > Pos[0])
				{
					Pos[0] = CoordId[0][i];
				}
				
				if(CoordId[1][i] > Pos[0])
				{
					Pos[0] = CoordId[1][i];
				}
			}
			Pos[1] = 0;
			for(i = 0;i < tmpQueryNum;i ++)
			{
				if(CoordId[0][i] > Pos[1] && CoordId[0][i] != Pos[0])
				{
					Pos[1] = CoordId[0][i];
				}
				
				if(CoordId[1][i] > Pos[1] && CoordId[1][i] != Pos[0])
				{
					Pos[1] = CoordId[1][i];
				}
			}
			
			// get base and quality one by one;
			CombineNum[0] = 0;
			CombineNum[1] = 0;
			for(j = 0;j < 2;j ++)
			{
				if(CoordId[j][NonDupQueryId] == Pos[0] && Pos[0])
				{
					InfoGet(LeftBaseArray,LeftQualArray,*(ReadShift + ReadId[j][NonDupQueryId]),CombineNum[0]);
					CombineNum[0] ++;
				}
				else if(CoordId[j][NonDupQueryId] == Pos[1] && Pos[1])
				{
					InfoGet(RightBaseArray,RightQualArray,*(ReadShift + ReadId[j][NonDupQueryId]),CombineNum[1]);
					CombineNum[1] ++;
				}
			}
			for(i = 0;i < tmpQueryNum;i ++)
			{
				if(i == NonDupQueryId)
				{
					continue;
				}
				
				for(j = 0;j < 2;j ++)
				{
					if(CoordId[j][i] == Pos[0] && Pos[0])
					{
						InfoGet(LeftBaseArray,LeftQualArray,*(ReadShift + ReadId[j][i]),CombineNum[0]);
						CombineNum[0] ++;
					}
					else if(CoordId[j][i] == Pos[1] && Pos[1])
					{
						InfoGet(RightBaseArray,RightQualArray,*(ReadShift + ReadId[j][i]),CombineNum[1]);
						CombineNum[1] ++;
					}
				}
			}
			
			// combine base and quality;
			RevisedFlag = 0;
			if(CombineNum[0])
			{
				for(i = 0; i < ReadLen;i ++)
				{
					for(j = 0;j < CombineNum[0];j ++)
					{
						TBase[j] = LeftBaseArray[j][i];
						TQual[j] = LeftQualArray[j][i];
					}
					RevisedFlag += BaseQualCombine(TBase,TQual,LeftCBase,LeftCQual,CombineNum[0],i);
				}
			}
			if(CombineNum[1])
			{
				for(i = 0; i < ReadLen;i ++)
				{
					for(j = 0;j < CombineNum[1];j ++)
					{
						TBase[j] = RightBaseArray[j][i];
						TQual[j] = RightQualArray[j][i];
					}
					RevisedFlag += BaseQualCombine(TBase,TQual,RightCBase,RightCQual,CombineNum[1],i);
				}
			}
			
			// output dup into bam file while non-dup into fq file for re-mapping;
			for(i = tmpFrom;i < tmpTo;i ++)
			{
				if(*(DupFlag + *(Index[Id2] + i)))
				{
					// diect output;
					OriginalPut(*(ReadShift + *(Index[Id2] + i)),1);
				}
				else if(RevisedFlag == 0)
				{
					OriginalPut(*(ReadShift + *(Index[Id2] + i)),0);
				}
				
				if(OutBuffId > MaxOutBuffSize)
				{
					fwrite(OutBuff,1,OutBuffId,fod);
					OutBuffId = 0;
				}
			}
			
			if(RevisedFlag)
			{
				for(i = 0;i < 2;i ++)
				{
					if(CoordId[i][NonDupQueryId])
					{
						if(CoordId[i][NonDupQueryId] == Pos[0])
						{
							// left seq;
							RevisedPut(*(ReadShift + ReadId[i][NonDupQueryId]),i,ReadId[i][NonDupQueryId],LeftCBase,LeftCQual,OriFlag[i][NonDupQueryId]);
						}
						else
						{
							// right seq;
							RevisedPut(*(ReadShift + ReadId[i][NonDupQueryId]),i,ReadId[i][NonDupQueryId],RightCBase,RightCQual,OriFlag[i][NonDupQueryId]);
						}
					}
					else
					{
						OriFlag[i][NonDupQueryId] |= 0x80;
						RevisedPut(*(ReadShift + ReadId[i][NonDupQueryId]),i,ReadId[i][NonDupQueryId],LeftCBase,LeftCQual,OriFlag[i][NonDupQueryId]);
					}
				}
				
				if(FqBuffId[0] > MaxOutBuffSize)
				{
					fwrite(FqBuff[0],1,FqBuffId[0],fodp);
					FqBuffId[0] = 0;
				}
				if(FqBuffId[1] > MaxOutBuffSize)
				{
					fwrite(FqBuff[1],1,FqBuffId[1],fodq);
					FqBuffId[1] = 0;
				}
			}
		}
		else
		{
			for(i = tmpFrom;i < tmpTo;i ++)
			{
				if(*(DupFlag + *(Index[Id2] + i)))
				{
					// diect output;
					OriginalPut(*(ReadShift + *(Index[Id2] + i)),1);
				}
				else
				{
					OriginalPut(*(ReadShift + *(Index[Id2] + i)),0);
				}
				
				if(OutBuffId > MaxOutBuffSize)
				{
					fwrite(OutBuff,1,OutBuffId,fod);
					OutBuffId = 0;
				}
			}
		}
		
		tmpFrom = tmpTo;
	}
	fclose(fid);
	
	if(OutBuffId)
	{
		fwrite(OutBuff,1,OutBuffId,fod);
		OutBuffId = 0;
	}
	if(DStrandFlag)
	{
		if(DcsBuffId)
		{
			fwrite(DcsBuff,1,DcsBuffId,foddcs);
		}
		fclose(foddcs);
	}
	if(tCombineFlag)
	{
		if(FqBuffId[0])
		{
			fwrite(FqBuff[0],1,FqBuffId[0],fodp);
		}
		if(FqBuffId[1])
		{
			fwrite(FqBuff[1],1,FqBuffId[1],fodq);
		}
		fclose(fodp);
		fclose(fodq);
		
		// fq to fq.gz;
		sprintf(cmd,"gzip %s",Fq1Path);
		system(cmd);
		sprintf(cmd,"gzip %s",Fq2Path);
		system(cmd);
		FqPathRevise();
		sleep(60);
	}
	
	MemoryFree2();
	MemoryFree3();
	
	return 1;
}

// ————————————————————————————————————
//
//   标记重复;
//
// ————————————————————————————————————
int DupMark()
{
	unsigned char tUMI[100],ReadGroup[200],cmd[10000];
	unsigned int i,j,tmpFrom,tmpTo,tmpNum,tmpId,tmpAId,tmpBId,MaxReadNum,tmpQueryId;
	
	// 初始化所有Dup标记位为0（非重复）;
	MemoryRequire3();
	for(i = 0;i < BamReadNum;i ++)
	{
		*(DupFlag + i) = 0;
	}
	memset(DupDistr,0,100000*sizeof(unsigned int));
	
	DupFlagId = 0;
	// 从5'端开始比较;
	MaxReadNum = 0;
	for(i = 0;i < BamReadNum;i ++)
	{
		tmpAId = *(Map1 + *(Index[Id1] + i)) >> 31;
		tmpBId = *(Map2 + *(Index[Id1] + i)) >> 7 & 0x01;
		tmpId = tmpAId + tmpBId;
		if(tmpId == 1 && tmpAId)
		{
			// which one should change 5' and 3' before comparation;
			*(Index[Id2] + MaxReadNum) = *(Index[Id1] + i) | 0x80000000;
		}
		else
		{
			*(Index[Id2] + MaxReadNum) = *(Index[Id1] + i);
		}
		MaxReadNum ++;
	}
	DupSort(0,MaxReadNum - 1,0);
	
	// 从3'端开始比较;
	MaxReadNum = 0;
	for(i = 0;i < BamReadNum;i ++)
	{
		tmpAId = *(Map1 + *(Index[Id1] + i)) >> 31;
		tmpBId = *(Map2 + *(Index[Id1] + i)) >> 7 & 0x01;
		tmpId = tmpAId + tmpBId;
		if(tmpId == 1 && tmpBId)
		{
			// which one should not change 5' and 3' before comparation;
			*(Index[Id2] + MaxReadNum) = *(Index[Id1] + i) | 0x80000000;
			MaxReadNum ++;
		}
		else if(tmpId < 2)
		{
			*(Index[Id2] + MaxReadNum) = *(Index[Id1] + i);
			MaxReadNum ++;
		}
	}
	DupSort(0,MaxReadNum - 1,1);
	
	// 对所有的相关reads按照Dup分组的标记顺序进行排序;
	DupFlagSort();
	
	TimeLog("Final recording ..");
	// 有UMI时，需要经过合并的过程;
	if(UmiFlag)
	{
		ReviseAndCombine(FilterFlag,CombineFlag);
		
		if(CombineFlag)
		{
			TimeLog("Remapping");
			// RG:Z (read group) confirm;
			RGConfirm(SamPath,ReadGroup);
			// fq.gz to sam;
			if(ReadGroup == NULL)
			{
				sprintf(cmd,"perl %s -fq1 %s -fq2 %s -b %s",Fq2SamScript,Fq1Path,Fq2Path,tmpSamPath);
			}
			else
			{
				sprintf(cmd,"perl %s -fq1 %s -fq2 %s -b %s -rg %s",Fq2SamScript,Fq1Path,Fq2Path,tmpSamPath,ReadGroup);
			}
			printf("[ Command Line ] %s\n",cmd);
			system(cmd);
			sleep(180);
			
			// adding re-mapped reads to the end;
			fid = fopen(tmpSamPath,"r");
			if(fid == NULL)
			{
				printf("File cannot be open: %s\n",tmpSamPath);
				exit(1);
			}
			LineEnd = 0;
			BuffSize = 0;
			if(FakeFlag)
			{
				while(LineCap())
				{
					if(BuffContent[LineStart] != '@')
					{
						for(i = LineStart;i < LineEnd;i ++)
						{
							OutBuff[OutBuffId] = BuffContent[i];
							OutBuffId ++;
						}
						OutBuff[OutBuffId] = '\n';
						OutBuffId ++;
						
						if(OutBuffId > MaxOutBuffSize)
						{
							fwrite(OutBuff,1,OutBuffId,fod);
							OutBuffId = 0;
						}
					}
				}
			}
			else
			{
				while(LineCap())
				{
					if(BuffContent[LineStart] != '@')
					{
						tmpId = 0;
						for(i = LineStart;i < LineEnd;i ++)
						{
							if(BuffContent[i] == '\t')
							{
								break;
							}
							
							if(BuffContent[i] == ':')
							{
								tmpId = OutBuffId;
							}
							
							OutBuff[OutBuffId] = BuffContent[i];
							OutBuffId ++;
						}
						
						tmpNum = 0;
						for(j = tmpId + 1;j < OutBuffId;j ++)
						{
							tUMI[tmpNum] = OutBuff[j];
							tmpNum ++;
						}
						
						OutBuffId = tmpId;
						for(i;i < LineEnd;i ++)
						{
							OutBuff[OutBuffId] = BuffContent[i];
							OutBuffId ++;
						}
						OutBuff[OutBuffId] = '\t';
						OutBuffId ++;
						for(i = 0;i <= UmiFlag;i ++)
						{
							OutBuff[OutBuffId] = UmiName[i];
							OutBuffId ++;
						}
						for(i = 0;i < tmpNum;i ++)
						{
							OutBuff[OutBuffId] = tUMI[i];
							OutBuffId ++;
						}
						OutBuff[OutBuffId] = '\n';
						OutBuffId ++;
						
						if(OutBuffId > MaxOutBuffSize)
						{
							fwrite(OutBuff,1,OutBuffId,fod);
							OutBuffId = 0;
						}
					}
				}
			}
			if(OutBuffId)
			{
				fwrite(OutBuff,1,OutBuffId,fod);
			}
			fclose(fid);
			fclose(fod);
			
			TimeLog("Clean after remapping");
			// fq1, fq2 and tmp sam deletion;
			sprintf(cmd,"rm %s",Fq1Path);
			system(cmd);
			sprintf(cmd,"rm %s",Fq2Path);
			system(cmd);
			sprintf(cmd,"rm %s",tmpSamPath);
			system(cmd);
			sleep(60);
		}
		else
		{
			fclose(fod);
		}
		
		// sort sam file in coord order;
		TimeLog("Bam sorting");
		sprintf(cmd,"%s sort -o %s %s",SamtoolsBin,OutPath,OutSamPath);
		printf("[ Command Line ] %s\n",cmd);
		system(cmd);
		sleep(300);
		sprintf(cmd,"rm %s",OutSamPath);
		system(cmd);
		sleep(60);
	}
	// 没有UMI时，只需要挑选碱基质量之和最高的reads，做好标记即可;
	else
	{
		tmpFrom = 0;
		while(tmpFrom < BamReadNum)
		{
			for(tmpTo = tmpFrom + 1;tmpTo < BamReadNum;tmpTo ++)
			{
				if(*(DupFlag + *(Index[Id2] + tmpTo)) != *(DupFlag + *(Index[Id2] + tmpFrom)))
				{
					break;
				}
			}
			
			QuerySort(tmpFrom,tmpTo - 1);
			DupCount(tmpFrom,tmpTo - 1);
			
			tmpQueryId = IdWithMaxQuality(tmpFrom,tmpTo - 1);
			MarkWithNonDup(tmpFrom,tmpTo - 1,tmpQueryId);
			
			tmpFrom = tmpTo;
		}
		
		FinalRevise();
	}
	TimeLog("Dup recoding");
	DupRecord();
	
	return 1;
}
// =======================================================================
// =======================================================================





// ___________________________________
//
//            Main Part
// ___________________________________
int main(int argc, char *argv[])
{
	unsigned char cmd[10000];
	time(&start);
	
	// 参数输入及初始化;
	ArgumentsInit(argc,argv);
	
	
	// Collect the reads info from sam;
	TimeLog("Reads info collect");
	ReadInfoCollect(SamPath);
	
	
	// Order by query name;
	TimeLog("Query name sort");
	QueryNameSort();
	
	
	// dup marking;
	TimeLog("Dup marking");
	DupMark();
	
	// rm 1st sam and transfer sorted sam to bam;
	TimeLog("Final bam transfering");
	sprintf(cmd,"rm %s",SamPath);
	system(cmd);
	if(!UmiFlag)
	{
		sprintf(cmd,"%s view -bh %s > %s",SamtoolsBin,OutSamPath,OutPath);
		system(cmd);
		sleep(300);
		sprintf(cmd,"rm %s",OutSamPath);
		system(cmd);
	}
	
	// time and rate logging;
	TimeLog("The end");
}