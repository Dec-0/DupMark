#!/usr/bin/perl
use strict;
use Getopt::Long;
use File::Basename;

my ($Fq1,$Fq2,$Sam,$Ref,$RGId,$HelpFlag,$time_begin);
my $HelpInfo = <<USAGE;

 Fq2Sam.pl
 Auther: zhangdong_xie\@foxmail.com

   This script is used to map fastqs and generate Sam.

 -fq1    Read1;
 -fq2    Read2;
 -b      Sam file;
 -rg     RG (read group, default: None)
 -ref    ( Optional ) Reference;
 
 -bin    List for searching of related bin or scripts;
 -h      Help infomation;

USAGE

GetOptions(
	'fq1=s' => \$Fq1,
	'fq2=s' => \$Fq2,
	'b=s' => \$Sam,
	'rg=s' => \$RGId,
	'ref:s' => \$Ref,
	'h!' => \$HelpFlag
) or die $HelpInfo;

if($HelpFlag || !$Fq1 || !$Fq2 || !$Sam)
{
	die $HelpInfo;
}
else
{
	$time_begin = time;
	if($time_begin)
	{
		my ($localtime_year,$localtime_month,$localtime_stamp);
		my @temp_time;
		
		@temp_time = localtime();
		$localtime_year = $temp_time[5] + 1900;
		$localtime_month = $temp_time[4] + 1;
		$localtime_stamp = $localtime_year . "-" . $localtime_month . "-" . $temp_time[3] . "-" . $temp_time[2] . "-" . $temp_time[1] . "-" . $temp_time[0];
		print "[ $localtime_year/$localtime_month/$temp_time[3] $temp_time[2]:$temp_time[1]:$temp_time[0] ] This script begins.\n";
	}
	
	
	die "No fastq1 : $Fq1\n" unless(-e $Fq1);
	die "No fastq2 : $Fq2\n" unless(-e $Fq2);
	$RGId = "None" if(!$RGId);
}

if($Fq1 && $Fq2 && $Sam)
{
	my $Reference = "/biobiggen/data/headQuarter/user/xiezhangdong/DB/Reference/hg38/ucsc.hg38.fasta";
	my $BwaBin = "/biobiggen/data/headQuarter/user/xiezhangdong/Softs/bwa-0.7.17/bwa";
	my $SamtoolsBin = "/biobiggen/data/headQuarter/user/xiezhangdong/Softs/samtools-1.11/bin/samtools";
	$Reference = $Ref if($Ref);
	print "[ Command Line ] $BwaBin mem -t 2 -M -R \'\@RG\\tID:$RGId\\tLB:lib1\\tPL:illumina\\tPU:Unit1\\tSM:ASHD753\\tCN:None\' $Reference $Fq1 $Fq2 | $SamtoolsBin view -h > $Sam\n";
	`$BwaBin mem -t 2 -M -R \'\@RG\\tID:$RGId\\tLB:lib1\\tPL:illumina\\tPU:Unit1\\tSM:ASHD753\\tCN:None\' $Reference $Fq1 $Fq2 | $SamtoolsBin view -h > $Sam`;
	
	printf "[ %.2fmin ] Done.\n",(time - $time_begin)/60;
}


######### Sub functions ##########
sub RandGen
{
	my $Num = $_[0];
	
	my @Char = ();
	for my $i (48 .. 57)
	{
		push @Char, chr($i);
	}
	for my $i (65 .. 90)
	{
		push @Char, chr($i);
	}
	for my $i (97 .. 122)
	{
		push @Char, chr($i);
	}
	
	my $Total = @Char;
	my $String = "";
	for my $i (1 .. $Num)
	{
		$String .= $Char[int(rand($Total))];
	}
	
	return $String;
}