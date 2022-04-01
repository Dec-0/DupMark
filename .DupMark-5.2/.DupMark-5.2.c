#include <stdio.h>
#include <stdlib.h>  // for file handle;
#include <getopt.h> // for argument;
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "BasicRelated.h"
#include "BamRelated.h"
#include "SortRelated.h"
#include "MemoryRelated.h"
#include "ConsensusRelated.h"

time_t start;
unsigned char InPath[1000],OutPath[1000],SamPath[1000],OutSamPath[1000],SamtoolsBin[1000],Fq2SamScript[1000],DupLogPath[1000],Fq1Path[1000],Fq2Path[1000],tmpSamPath[1000],UmiName[100];
unsigned char **GeneralPS4Char,*Info4MapFullLeftChr,*Info4MapFullRightChr,*Info4MapLeftChr,*Info4MapRightChr,*Info4Index;
unsigned short **GeneralPS4Short,*Info4MapFullCigar;
unsigned int MapReadsNum,MapReadsPairNum,MaxArraySize,MaxPairArraySize,MinReads,ReadLen,UmiSize,HUmiSize;
unsigned int UmiFlag,DStrandFlag,CombineFlag,FilterFlag,FakeFlag,DebugFlag,IDFormatFlag,BamGenJumpFlag,DebugFlag;
unsigned int **GeneralPS4Int,**Index4Int,*SortInfo4ReadsID,*GroupInfo4ReadsID,*Info4MapFullLeftPos,*Info4MapFullRightPos,*Info4MapLeftPos,*Info4MapRightPos,*GroupInfo4Map,*SortInfo4MapFull,*GroupInfo4MapFull,*Info4BQ;
unsigned long **GeneralPS4Long,*Info4IDFull,*Info4Shift,*Info4UMISeq;


// ———————————————
// 命令行参数输入;
unsigned int OptGet(int argc, char *argv[])
{
	unsigned char InFile[10] = "infile";
	unsigned char OutFile[10] = "outfile";
	unsigned char Double[10] = "double";
	unsigned char Min[10] = "min";
	unsigned char Fake[10] = "fake";
	unsigned char UmiTag[10] = "umi";
	unsigned char Jump[10] = "jump";
	unsigned char Save[10] = "save";
	unsigned char Merge[10] = "Merge";
	unsigned char Samtools[10] = "S";
	unsigned char Fq2Sam[10] = "Fq2Sam";
	unsigned char Help[5] = "help";
	unsigned char Debug[5] = "b";
	unsigned char OptHash[20],BaseName[1000];
	unsigned char Info[5000] = "\n\n\
 DupMark-5.2   For Duplicates Marking and Consensus Making.\n\
 用于bam去重及UMI条件下的reads合并（SSCS、DCS ConsensusMaking）\n\n\
 和v4.0相比，新增参数“-g”，用于输出reads'ID的详细分组信息\n\
 同时修改了一个bug，非Illumina的reads格式保存中本应存入tY的存入了tX（虽然对BGI格式没有影响，因为只有23位，没有超过24位）\n\n\
 和v4.2版本相比，精细了内存占用规则，减少了总的内存占用量，代码几乎重写。但是该版本只是显示了标记重复，还没有涉及到序列合并，等后续有时间了再完成。\n\
 另外，新增了对UMI碱基中N的简并处理（在此前版本中N和ATCG是不同的）\n\
 和v5.1版本相比，提高了在不生成bam的条件下的处理速度\n\n\
 Last revised date: 2021.10.12\n\
 Contact:zhangdong_xie@foxmail.com\n\n\
 -i/-infile   ( Required ) Bam file prepare to be dup marked.\n\n\
 -o/-outfile  ( Optional ) Dup-marked bam file.\n\
 -u/-umi      ( Optional ) For the tag stamp for UMI which will add \":\" automatically (:Z should be included, like RX:Z).\n\
 -d/-double   ( Optional ) For dual UMI only which will make DCS, otherwise only SSCS (only -d, default: single strand UMI mode).\n\
 -m/-min      ( Optional ) The minimal number of reads for SSCS or DCS making (default: 3).\n\
 -f/fake      ( Optional ) Treat it with umi even if there was no umi.\n\
                           该参数主要用于在没有UMI的条件下同样进行序列合并。\n\
 -j/-jump     ( Optional ) If there was no need to generate de-dupped bam.\n\
 -s/-save     ( Optional ) If there was no need to filter out the un-paired reads in SSCS or DCS (only -s, default: filtering, only effective when there was UMI).\n\
 -M/-Merge    ( Optional ) If there was need to merge base and quality in SSCS and DCS combination (only -M, default: no change).\n\
 -S           ( Optional ) Directory for samtools (default: samtools).\n\
 -Fq2Sam      ( Optional ) Directory for Fq2SamScript (default:CurrentFolder/Fq2Sam.pl).\n\
 -b           ( Optional ) Flag for Debug.\n\
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
	// OptHash用于记录对应序号的参数是否有指定;
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
				
				if(OptHash[1])
				{
					printf("[ Error ] Argument -i/-infile specifed more than once.\n");
					exit(1);
				}
				
				// must followed with value;
				i ++;
				StringCopy2String(InPath,argv[i]);
				OptHash[1] = 1;
			}
			else if(argv[i][j] == OutFile[0])
			{
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
				
				if(OptHash[2])
				{
					printf("[ Error ] Argument -o/-outfile specifed more than once.\n");
					exit(1);
				}
				
				// must followed with value;
				i ++;
				StringCopy2String(OutPath,argv[i]);
				OptHash[2] = 1;
			}
			else if(argv[i][j] == UmiTag[0])
			{
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
				
				if(OptHash[3])
				{
					printf("[ Error ] Argument -u/-umi specifed more than once.\n");
					exit(1);
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
				StringCopy2String(UmiName,argv[i]);
				UmiFlag = StringAdd2String(UmiName,":");
				UmiFlag -= 1;
				OptHash[3] = 1;
			}
			else if(argv[i][j] == Double[0])
			{
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
				
				if(OptHash[4])
				{
					printf("[ Error ] Argument -d/-double specifed more than once.\n");
					exit(1);
				}
				
				OptHash[4] = 1;
			}
			else if(argv[i][j] == Jump[0])
			{
				// match with "jump";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 4] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 4;k ++)
						{
							if(argv[i][k] != Jump[tmp])
							{
								printf("[ Error ] Wrong format for -jump.%s",Info);
								exit(1);
							}
							tmp ++;
						}
					}
					else
					{
						printf("[ Error ] Wrong format for -j.%s",Info);
						exit(1);
					}
				}
				
				if(OptHash[12])
				{
					printf("[ Error ] Argument -j/-jump specifed more than once.\n");
					exit(1);
				}
				
				OptHash[12] = 1;
				BamGenJumpFlag = 1;
			}
			else if(argv[i][j] == Save[0])
			{
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
				
				if(OptHash[5])
				{
					printf("[ Error ] Argument -s/-save specifed more than once.\n");
					exit(1);
				}
				
				OptHash[5] = 1;
			}
			else if(argv[i][j] == Merge[0])
			{
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
				
				if(OptHash[6])
				{
					printf("[ Error ] Argument -m/-merge specifed more than once.\n");
					exit(1);
				}
				
				OptHash[6] = 1;
			}
			else if(argv[i][j] == Samtools[0])
			{
				if(argv[i][j + 1])
				{
					printf("[ Error ] Wrong format for -S.%s",Info);
					exit(1);
				}
				
				if(OptHash[7])
				{
					printf("[ Error ] Argument -S specifed more than once.\n");
					exit(1);
				}
				
				i ++;
				StringCopy2String(SamtoolsBin,argv[i]);
				OptHash[7] = 1;
			}
			else if(argv[i][j] == Fq2Sam[0])
			{
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
				
				if(OptHash[8])
				{
					printf("[ Error ] Argument -F/-Fq2Sam specifed more than once.\n");
					exit(1);
				}
				
				i ++;
				StringCopy2String(Fq2SamScript,argv[i]);
				OptHash[8] = 1;
			}
			else if(argv[i][j] == Min[0])
			{
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
				
				if(OptHash[9])
				{
					printf("[ Error ] Argument -m/-min specifed more than once.\n");
					exit(1);
				}
				
				// must followed with value;
				i ++;
				MinReads = String2Int(argv[i]);
				OptHash[9] = 1;
			}
			else if(argv[i][j] == Fake[0])
			{
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
				
				if(OptHash[10])
				{
					printf("[ Error ] Argument -f/-fake specifed more than once.\n");
					exit(1);
				}
				
				OptHash[10] = 1;
				FakeFlag = 1;
				// 假如需要在不依赖UMI的条件下进行序列合并;
				UmiFlag = 4;
				DStrandFlag = 1;
				// Fake模式下统一分配同样的UMI序列，而且是双侧UMI，此时不用指定DStrandFlag;
				UmiSize = 2;
				HUmiSize = 1;
			}
			else if(argv[i][j] == Debug[0])
			{
				// match with "group";
				if(argv[i][j + 1])
				{
					if(argv[i][j + 5] == 0)
					{
						tmp = 1;
						for(k = j + 1;k < j + 5;k ++)
						{
							if(argv[i][k] != Debug[tmp])
							{
								printf("[ Error ] Wrong format for -d.%s",Info);
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
				
				if(OptHash[11])
				{
					printf("[ Error ] Argument -d specifed more than once.\n");
					exit(1);
				}
				
				OptHash[11] = 1;
				// 假如只统计reads的分组信息；
				DebugFlag = 1;
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
		StringCopy2String(OutPath,InPath);
		StringSuffixReplace(OutPath,"bam","dedup.bam");
	}
	// 确定输入bam解压后的sam文件路径;
	StringDir(SamPath,OutPath);
	StringBaseName(BaseName,InPath);
	StringAdd2String(SamPath,BaseName);
	StringSuffixReplace(SamPath,"bam","sam");
	// 确定输出的sam文件路径;
	StringCopy2String(OutSamPath,OutPath);
	StringSuffixReplace(OutSamPath,"bam","sam");
	// 用于记录TagFamilySize的日志文件;
	StringCopy2String(DupLogPath,OutPath);
	StringSuffixReplace(DupLogPath,"bam","distr.xls");
	// 需要make consensus时，相关的参数（比如用于重比对的fq1、fq2、临时sam文件）;
	if(OptHash[3] || OptHash[10])
	{
		StringCopy2String(Fq1Path,OutPath);
		StringSuffixReplace(Fq1Path,".bam","_R1.fq");
		
		StringCopy2String(Fq2Path,OutPath);
		StringSuffixReplace(Fq2Path,".bam","_R2.fq");
		
		StringCopy2String(tmpSamPath,OutPath);
		StringSuffixReplace(tmpSamPath,"bam","tmp.sam");
	}
	
	
	// UMI模式（SSCS或者DCS，默认DCS，后者会对两侧UMI序列交换比较）;
	if(OptHash[4])
	{
		DStrandFlag = 1;
	}
	// 是否需要进行序列合并，或者只是简单地标记重复;
	if(OptHash[6])
	{
		CombineFlag = 1;
	}
	// 是否需要过滤在ConsensusMaking过程中未满足条件的reads（默认过滤）;
	if(OptHash[5])
	{
		FilterFlag = 0;
	}
	
	
	// fq2sam.pl脚本路径;
	if(OptHash[8] == 0 && CombineFlag)
	{
		StringDir(Fq2SamScript,argv[0]);
		StringAdd2String(Fq2SamScript,"Fq2Sam.pl");
	}
	
	return 1;
}

// —————————
// 默认参数；
unsigned int DefalutArg()
{
	// 默认没有UMI；
	UmiFlag = 0;
	// 默认只支持SSCS而不支持DCS；
	DStrandFlag = 0;
	// 默认不进行ConsensusMaking；
	CombineFlag = 0;
	// 默认SSCS或DCS时最少的reads支持数；
	MinReads = 3;
	// 默认在ConsensusMaking过程中会过滤掉不符合条件的Reads;
	FilterFlag = 1;
	// 默认关闭Fake模式;
	FakeFlag = 0;
	// 默认的samtools;
	StringCopy2String(SamtoolsBin,"samtools");
	// 默认非Debug模式；
	DebugFlag = 0;
	// 默认是需要生成去重后bam的；
	BamGenJumpFlag = 0;
	// For Debug Only;
	DebugFlag = 0;
}

// ———————————————
// 指针初始化相关；
unsigned int MemoryRequireInitialOfInfo4IDFull()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Long = (unsigned long **)malloc(ArraySize * sizeof(unsigned long *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Long %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfInfo4IDFull(MaxArraySize,GeneralPS4Long);
	Info4IDFull = *(GeneralPS4Long + 0);
	
	free(GeneralPS4Long);
	
	return 1;
}
unsigned int MemoryRequireInitialOfInfo4Map()
{
	unsigned int ArraySize = 2;
	
	if((GeneralPS4Char = (unsigned char **)malloc(ArraySize * sizeof(unsigned char *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Char %u).\n",ArraySize);
		exit(1);
	}
	if((GeneralPS4Int = (unsigned int **)malloc(ArraySize * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Int %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfInfo4Map(MaxPairArraySize,GeneralPS4Char,GeneralPS4Int);
	Info4MapLeftChr = *(GeneralPS4Char + 0);
	Info4MapLeftPos = *(GeneralPS4Int + 0);
	Info4MapRightChr = *(GeneralPS4Char + 1);
	Info4MapRightPos = *(GeneralPS4Int + 1);
	
	free(GeneralPS4Char);
	free(GeneralPS4Int);
	
	return 1;
}
unsigned int MemoryRequireInitialOfSort(unsigned int Size)
{
	unsigned int ArraySize = 2;
	
	if((Index4Int = (unsigned int **)malloc(ArraySize * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( Index4Int %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfSort(Size,Index4Int);
	
	return 1;
}
unsigned int MemoryRequireInitialOfInfo4MapFull()
{
	unsigned int ArraySize = 2;
	
	if((GeneralPS4Char = (unsigned char **)malloc(ArraySize * sizeof(unsigned char *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Char %u).\n",ArraySize);
		exit(1);
	}
	if((GeneralPS4Short = (unsigned short **)malloc(ArraySize * sizeof(unsigned short *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Short %u).\n",ArraySize);
		exit(1);
	}
	if((GeneralPS4Int = (unsigned int **)malloc(ArraySize * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Int %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfInfo4MapFull(MaxArraySize,GeneralPS4Char,GeneralPS4Short,GeneralPS4Int);
	Info4MapFullCigar = *(GeneralPS4Short + 0);
	Info4MapFullLeftChr = *(GeneralPS4Char + 0);
	Info4MapFullLeftPos = *(GeneralPS4Int + 0);
	Info4MapFullRightChr = *(GeneralPS4Char + 1);
	Info4MapFullRightPos = *(GeneralPS4Int + 1);
	
	free(GeneralPS4Char);
	free(GeneralPS4Short);
	free(GeneralPS4Int);
	
	return 1;
}
unsigned int MemoryRequireInitialOfIndex()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Char = (unsigned char **)malloc(ArraySize * sizeof(unsigned char *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Char %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfIndex(MaxArraySize,GeneralPS4Char);
	Info4Index = *(GeneralPS4Char + 0);
	
	free(GeneralPS4Char);
	
	return 1;
}
unsigned int MemoryRequireInitialOfMapGroup()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Int = (unsigned int **)malloc(ArraySize * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Int %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfMapGroup(MaxPairArraySize,GeneralPS4Int);
	GroupInfo4Map = *(GeneralPS4Int + 0);
	
	free(GeneralPS4Int);
	
	return 1;
}
unsigned int MemoryRequireInitialOfMapGroupFull()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Int = (unsigned int **)malloc(ArraySize * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Int %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfMapGroupFull(MaxArraySize,GeneralPS4Int);
	GroupInfo4MapFull = *(GeneralPS4Int + 0);
	
	free(GeneralPS4Int);
	
	return 1;
}
unsigned int MemoryRequireInitialOfInfo4Shift()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Long = (unsigned long **)malloc(ArraySize * sizeof(unsigned long *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Long %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfInfo4Shift(MaxArraySize,GeneralPS4Long);
	Info4Shift = *(GeneralPS4Long + 0);
	
	free(GeneralPS4Long);
	
	return 1;
}
unsigned int MemoryRequireInitialOfInfo4UMISeq()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Long = (unsigned long **)malloc(ArraySize * sizeof(unsigned long *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Long %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfInfo4UMISeq(MaxPairArraySize,GeneralPS4Long);
	Info4UMISeq = *(GeneralPS4Long + 0);
	
	free(GeneralPS4Long);
	
	return 1;
}
unsigned int MemoryRequireInitialOfInfo4BQ()
{
	unsigned int ArraySize = 1;
	
	if((GeneralPS4Int = (unsigned int **)malloc(ArraySize * sizeof(unsigned int *))) == NULL)
	{
		printf("[ Error ] Malloc memory unsuccessfully ( GeneralPS4Long %u).\n",ArraySize);
		exit(1);
	}
	
	MemoryRequireOfInfo4BQ(MaxPairArraySize,GeneralPS4Int);
	Info4BQ = *(GeneralPS4Int + 0);
	
	free(GeneralPS4Int);
	
	return 1;
}


// ———————————
// 初始化准备;
unsigned int ArgumentsInit(int argc, char *argv[])
{
	unsigned char cmd[5000],Other[5000];
	
	// 参数输入及初始化;
	DefalutArg();
	OptGet(argc,argv);
	
	// bam解压为sam (20G bam to 220G sam, 需要大概 30 mins);
	// 300G bam to sam 需要大概。
	TimeLog(start,"Format transform from bam to sam ..");
	sprintf(cmd,"%s view -h %s > %s",SamtoolsBin,InPath,SamPath);
	sprintf(Other,"CommandLine:  %s",cmd);
	TimeLog(start,Other);
	system(cmd);
	TimeLog(start,"Format transform done");
	
	// Reads读长
	ReadLen = ReadLenConfirmFromSam(SamPath);
	sprintf(cmd,"Read length: %u",ReadLen);
	TimeLog(start,cmd);
	// ReadsID的模式，比如Illumina、BGI。
	IDFormatFlag = IDFormatConfirmFromSam(SamPath);
	if(IDFormatFlag == 1)
	{
		TimeLog(start,"Illumina format for reads' ID");
	}
	else if(IDFormatFlag == 2)
	{
		TimeLog(start,"BGI format for reads' ID");
	}
	else
	{
		TimeLog(start,"Unknown format for reads' ID");
	}
	
	
	// 有UMI时；
	if(UmiFlag && !FakeFlag)
	{
		UmiSize = UmiSizeConfirm(SamPath,UmiName);
		if(UmiSize == 0)
		{
			TimeLog(start,"No UMI characters, converting to normal dupmark procedure");
			
			UmiFlag = 0;
			DStrandFlag = 0;
			CombineFlag = 0;
		}
		else
		{
			sprintf(cmd,"The size of UMI: %u",UmiSize);
			TimeLog(start,cmd);
		}
		
		// 双侧UMI时需要UMI序列长度为偶数；
		if(DStrandFlag && UmiSize % 2)
		{
			printf("[ Error ] The number of umi is un-even: %u.\n",UmiSize);
			exit(1);
		}
		if(DStrandFlag)
		{
			HUmiSize = UmiSize / 2;
			sprintf(cmd,"The size of half UMI: %u",HUmiSize);
			TimeLog(start,cmd);
		}
	}
	
	return 1;
}


// ——————————————————————————————————————
// 数组长度从比对数量降低到Reads对的数量;
unsigned int NarrowOfInfo4MapFull()
{
	unsigned int i;
	
	// 申请最多一半内存G*10Bytes，总内存为40GBytes；
	MemoryRequireInitialOfInfo4Map();
	for(i = 0;i < MapReadsNum;i ++)
	{
		*(Info4MapLeftChr + *(GroupInfo4ReadsID + i) - 1) = *(Info4MapFullLeftChr + i);
		*(Info4MapLeftPos + *(GroupInfo4ReadsID + i) - 1) = *(Info4MapFullLeftPos + i);
		*(Info4MapRightChr + *(GroupInfo4ReadsID + i) - 1) = *(Info4MapFullRightChr + i);
		*(Info4MapRightPos + *(GroupInfo4ReadsID + i) - 1) = *(Info4MapFullRightPos + i);
	}
	
	// 释放2G*10Bytes，总内存为20GBytes；
	MemoryFreeOfInfo4MapFull(Info4MapFullLeftChr,Info4MapFullLeftPos,Info4MapFullRightChr,Info4MapFullRightPos);
	
	return 1;
}

// ——————————————————————————————————————————————
// 1. 获取ReadsID信息并进行排序分组，保留排序和分组信息;
unsigned int ReadsIDGroup()
{
	unsigned char cmd[10000];
	unsigned int HeaderLineCount;
	
	// Bam中的行数;
	MapReadsNum = FileLineCount(SamPath);
	HeaderLineCount = SamHeadLineCount(SamPath);
	MapReadsNum = MapReadsNum - HeaderLineCount;
	sprintf(cmd,"The number of lines in bam: %u",MapReadsNum);
	TimeLog(start,cmd);
	// 需要的数组最大下标
	MaxArraySize = (unsigned int)(MapReadsNum * 1.02);
	sprintf(cmd,"Max ArraySize: %u",MaxArraySize);
	TimeLog(start,cmd);
	
	// 申请存储ReadsID信息的内存空间（2G*8Bytes，G指代Reads对数）；
	MemoryRequireInitialOfInfo4IDFull();
	// 获取具体的ReadsID信息；
	TimeLog(start,"Info acquiring of ReadsID begin");
	InfoGet4ReadsID(SamPath,IDFormatFlag,Info4IDFull);
	TimeLog(start,"Info acquiring of ReadsID done");
	
	// 对ReadsID信息进行排序、分组；
	// 申请排序的内存空间（2G*4*2Bytes），剩余32GBytes；
	MemoryRequireInitialOfSort(MaxArraySize);
	// 排序和分组；
	MapReadsPairNum = SortAndGroup4Long(Info4IDFull,Index4Int,MapReadsNum);
	sprintf(cmd,"Total number of reads' pair: %u",MapReadsPairNum);
	TimeLog(start,cmd);
	MaxPairArraySize = (unsigned int)(MapReadsPairNum * 1.02);
	// 修改指针名；
	SortInfo4ReadsID = Index4Int[0];
	GroupInfo4ReadsID = Index4Int[1];
	// 释放探针，但未释放内存；
	MemoryFreeOfSortBase(Index4Int);
	
	// 释放2G*8Bytes，剩余内存占用约 2G*8Bytes；
	MemoryFreeOfInfo4IDFull(Info4IDFull);
	
	return 1;
}

// —————————————————————————————
// 2. 获取比对信息并进行统一化操作;
unsigned int MapInfoInitial()
{
	// 申请存储比对信息的内存空间（2G*12Bytes），剩余内存为40GBytes；
	MemoryRequireInitialOfInfo4MapFull();
	// 获取具体的比对信息（500G的bam大约耗时1h）；
	InfoGet4Map(SamPath,Info4MapFullCigar,Info4MapFullLeftChr,Info4MapFullLeftPos,Info4MapFullRightChr,Info4MapFullRightPos);
	TimeLog(start,"Info acquiring of MappingInfo done");
	
	// 根据ReadsID排序信息逐个处理ReadsID，进行统一操作；
	MapInfoUniform(MapReadsNum,SortInfo4ReadsID,GroupInfo4ReadsID,Info4MapFullCigar,Info4MapFullLeftChr,Info4MapFullLeftPos,Info4MapFullRightChr,Info4MapFullRightPos,DebugFlag);
	TimeLog(start,"Uniform of MappingInfo done");
	// 消除Cigar信息，2G*2Bytes，总内存为36GBytes;
	MemoryFreeOfCigar(Info4MapFullCigar);
	// 申请空间2G*1Bytes，记录原始Read1和Read2信息，剩余内存为38GBytes；
	MemoryRequireInitialOfIndex();
	InfoRecord4Index(MapReadsNum,Info4Index,SortInfo4ReadsID,GroupInfo4ReadsID,Info4MapFullLeftChr,Info4MapFullLeftPos,Info4MapFullRightChr,Info4MapFullRightPos);
	// 释放ReadsID排序信息内存空间2G*4Bytes，剩余内存为30GBytes；
	MemoryFreeOfSortInfo4ReadsID(SortInfo4ReadsID);
	TimeLog(start,"Recording of reads' index done");
	// 2G*10Bytes转G*10Bytes，峰值40GBytes，剩余内存为20GBytes；
	NarrowOfInfo4MapFull();
	TimeLog(start,"Memory narrow down finished");
	
	return 1;
}

// —————————————————————————
// 3. 对比对信息进行排序及分组;
unsigned int MapInfoGroup()
{
	// 对比对信息进行排序、分组，以期减少内存占用；
	// 申请排序的内存空间（G*4*2Bytes），总内存为28GBytes；
	MemoryRequireInitialOfSort(MaxPairArraySize);
	// 申请标记重复的内存空间（G*4Bytes），总内存为32GBytes；
	MemoryRequireInitialOfMapGroup();
	// 排序和分组；
	SortAndGroup4MapInfo(start,Info4MapLeftChr,Info4MapLeftPos,Info4MapRightChr,Info4MapRightPos,Index4Int,GroupInfo4Map,MapReadsPairNum,DebugFlag);
	// 释放10GBytes，剩余内存占用约 22GBytes；
	MemoryFreeOfInfo4Map(Info4MapLeftChr,Info4MapLeftPos,Info4MapRightChr,Info4MapRightPos);
	// 释放G*8GBytes，总内存为14GBytes；
	MemoryFreeOfSortFull(Index4Int);
	TimeLog(start,"Sort and groupping of MappingInfo done");
	
	// 比对信息分组完成后，没必要再保留aa vs a0的差异记录了；
	
	// 根据比对分组信息，扩大到所有reads；
	// 申请内存2G*4Bytes，总内存22GBytes；
	MemoryRequireInitialOfMapGroupFull();
	ExpandMapGroup(GroupInfo4ReadsID,GroupInfo4Map,GroupInfo4MapFull,MapReadsNum);
	// 释放4GBytes，总内存为18GBytes；
	MemoryFreeOfMapGroup(GroupInfo4Map);
	TimeLog(start,"Memory Restore done");
	
	// 对完整的比对位置进行排序、分组；
	// 申请排序的内存空间（2G*4*2Bytes），总内存为34GBytes；
	MemoryRequireInitialOfSort(MaxArraySize);
	// 排序和分组；
	SortAndGroup4Int(GroupInfo4MapFull,Index4Int,MapReadsNum);
	// 保留排序信息，释放8GBytes，总内存为26GBytes；
	SortInfo4MapFull = Index4Int[0];
	MemoryFreeOfSortTwo(Index4Int);
	TimeLog(start,"Sort for intact MappingInfo done");
	
	return 1;
}


// ————————————————————————————————
// 4. 读入文件偏移量，进行去重操作;
unsigned int DupGroup()
{
	if(UmiFlag)
	{
		// 真UMI模式 或 假UMI模式（无UMI但需要Making Consensus）；
		if(BamGenJumpFlag)
		{
			// 不需要生成Bam文件时，只需要读入UMI信息；
			// 申请存储比对信息的内存空间（G*8Bytes），总内存为34GBytes；
			TimeLog(start,"Begin info acquiring of UMI");
			MemoryRequireInitialOfInfo4UMISeq();
			// 假如有假UMI标记，则不用真实读取。
			UMISeqAllGetFromSam(SamPath,GroupInfo4ReadsID,Info4Index,UmiName,HUmiSize,Info4UMISeq,MapReadsPairNum,FakeFlag);
			TimeLog(start,"Info acquiring of UMI done");
			// 不需要生成bam时根本不需要BQ信息；
			//MemoryRequireInitialOfInfo4BQ();
			//BQAllGetFromSam(SamPath,GroupInfo4ReadsID,Info4Index,Info4BQ,MapReadsPairNum);
			
			DupCountWithUMI(start,MapReadsNum,GroupInfo4ReadsID,SortInfo4MapFull,GroupInfo4MapFull,Info4Index,Info4UMISeq,DupLogPath,HUmiSize,FakeFlag);
		}
		else
		{
			// 需要生成Bam文件时，老实读入偏移量逐个处理。
			// 申请存储比对信息的内存空间（2G*8Bytes），总内存为42GBytes；
			MemoryRequireInitialOfInfo4Shift();
			// 获取具体的比对信息（假如非UMI条件且不需要生成或者修改Bam，则不需要获取偏移量信息）；
			InfoGet4Shift(SamPath,Info4Shift);
			TimeLog(start,"Info acquiring of Pointshift done");
			
			// 需要同时配合处理bam，Making Consensus；
			DupMarkWithUMI(start,SamPath,MapReadsNum,Info4Shift,GroupInfo4ReadsID,SortInfo4MapFull,GroupInfo4MapFull,Info4Index,DupLogPath,UmiName,HUmiSize,FakeFlag);
		}
	}
	else
	{
		// 无UMI条件下，只需要读入BQ信息参与去重即可；
		TimeLog(start,"Begin info acquiring of BQ");
		MemoryRequireInitialOfInfo4BQ();
		BQAllGetFromSam(SamPath,GroupInfo4ReadsID,Info4Index,Info4BQ,MapReadsPairNum);
		TimeLog(start,"Info acquiring of BQ done");
		
		// 确定需要标记的编号；
		DupMarkWithOutUMI(start,MapReadsNum,GroupInfo4ReadsID,SortInfo4MapFull,GroupInfo4MapFull,Info4Index,Info4BQ,BamGenJumpFlag,DupLogPath);
		TimeLog(start,"Info acquiring of Duplication done");
		
		if(!BamGenJumpFlag)
		{
			// 修改标记位；
			FlagRevise(SamPath,OutSamPath,Info4Index);
			TimeLog(start,"Real marking done");
		}
	}
	
	return 1;
}



// ——————————
// Main Part；
int main(int argc, char *argv[])
{
	unsigned char cmd[10000],Other[10000];
	
	// 参数输入及初始化;
	// 注意169G数据量，1.1G行的bam，读一遍需要大约30min;
	time(&start);
	ArgumentsInit(argc,argv);
	
	// 数据预处理;
	TimeLog(start,"Info collection of ReadsID begins");
	ReadsIDGroup();
	TimeLog(start,"Info collection Of MappingInfo begins");
	MapInfoInitial();
	TimeLog(start,"Group for MappingInfo");
	MapInfoGroup();
	
	// 去重开始;
	TimeLog(start,"Duplicates marking begins");
	DupGroup();
	
	if(!BamGenJumpFlag)
	{
		// sam转换成bam
		TimeLog(start,"Format transform from sam to bam ..");
		sprintf(cmd,"%s sort -o %s %s",SamtoolsBin,OutPath,OutSamPath);
		sprintf(Other,"CommandLine:  %s",cmd);
		TimeLog(start,Other);
		system(cmd);
		// 删除相关sam文件；
		sprintf(cmd,"rm %s",OutSamPath);
		sprintf(Other,"CommandLine:  %s",cmd);
		TimeLog(start,Other);
		system(cmd);
		TimeLog(start,"Format transform done");
	}
	sprintf(cmd,"rm %s",SamPath);
	sprintf(Other,"CommandLine:  %s",cmd);
	TimeLog(start,Other);
	system(cmd);
	
	// 日志;
	TimeLog(start,"The end");
}