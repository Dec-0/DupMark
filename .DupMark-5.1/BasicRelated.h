#ifndef _BASICRELATED_H
#define _BASICRELATED_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// 文件相关 ========================================
// 按行读取（初次调用时需要将LineEnd和BuffSize均置为0）;
// 使用示例“MultiLineCap(fid,BuffContent,MaxBuffSize,&BuffSize,&LineStart,&LineEnd)”；
unsigned char MultiLineCap(FILE *fid,unsigned char *BuffContent,unsigned int MaxBuffSize,unsigned int *BuffSize,unsigned int *LineStart,unsigned int *LineEnd);

// 确定文件行数；
unsigned int FileLineCount(unsigned char *File);
unsigned int SamHeadLineCount(unsigned char *File);

// 日志 ===========================================
// 用于输出日志记录
unsigned int TimeLog(time_t start,unsigned char *String);

// 数字与字符的转换 ===============================
// 用于将字符型的数字转换为纯数字;
unsigned char String2Char(unsigned char *String);
unsigned int String2Int(unsigned char *String);
unsigned long String2Long(unsigned char *String);

// 用于将纯数字转换为字符型的数字;
unsigned int Int2String(unsigned int Num, unsigned char *Char);


// 字符串处理相关 ========================================
// 用于后者字符串内容 赋值到 前者中，并返回字符串长度;
unsigned int StringCopy2String(unsigned char *StringTo, unsigned char *StringFrom);

unsigned int IntCopy2String(unsigned char *StringTo, unsigned int Num);

// 用于将后者字符串内存 连接到 前者字符串末尾，并返回拼接完成后的总长度;
unsigned int StringAdd2String(unsigned char *StringMain, unsigned char *String);

unsigned int CharAdd2String(unsigned char *StringMain, unsigned char Char);
unsigned int CharAdd2StringSimple(unsigned char *StringMain, unsigned char Char, unsigned int Id);

unsigned int IntAdd2String(unsigned char *StringMain, unsigned int Num);
unsigned int IntAdd2StringSimple(unsigned char *StringMain, unsigned int Num, unsigned int Id);

// 用于将后者的dirname部分赋值给前者（包括最后一个“/”），并返回字符串长度;
unsigned int StringDir(unsigned char *StringDir, unsigned char *String);

// 用于将后者的basename部分赋值给前者，并返回字符串长度;
unsigned int StringBaseName(unsigned char *BaseName, unsigned char *String);

// 用于替换字符串行首、末尾；
unsigned int StringPrefixReplace(unsigned char *String,unsigned char *CurrPrefix,unsigned char *RePrefix);
unsigned int StringSuffixReplace(unsigned char *String,unsigned char *CurrSuffix,unsigned char *ReSuffix);

// 用于判断字符串是否相等;
unsigned int IfStringEqual(unsigned char *StringA,unsigned char *StringB);
unsigned int IfStringBegin(unsigned char *StringMain,unsigned char *StringPrefix);
unsigned int IfStringHas(unsigned char *String,unsigned char Char);

// 碱基和数字的转换 ===============
unsigned int Base2Bit(unsigned char Base);
unsigned char Bit2Base(unsigned char Bit);

// 染色体号转换成编号 =============
unsigned char Chr2Char(unsigned char *String,unsigned char ReversFlag);

#endif
