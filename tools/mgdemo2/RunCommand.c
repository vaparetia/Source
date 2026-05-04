/******************************************************************************
 * System	: DemoAct
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Kelnel
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include "Kernel.h"
#include "RunCommand.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */
char* lppMakeCommand[] = {
	"cd scn; make;",
	"cd scn; make stage;",
	"cd source; make link;",
	"cd source; make go;",
	"" };

char* lppMakeLinkOneStage[] = {
	"cd source; make MGS2STAGE=",
	" link;",
	"" };

char* lppMakeHDModule[] = {
	"cd source; make MGS2STAGE=",
	" link_hdu;",
	"cd source; ee-strip",
	"cd source; mkcdimg",
	"" };


enum {
	RUN_SCN,
	RUN_STAGE,
	RUN_LINK,
	RUN_GO,
	};

/******************************************************************************
 * make などを行う
 */
int RunCommand(			/* 0 : 異常 */
	int   nCommand,		/* make type */
	char* lpszTarget,
	char* lpszResident,
	char* lpszCDRomImgDirectory)
{
	int  i;
	char szCommand[_MAX_TEXT];

	if((nCommand & MGDEMO_DEMO) || (nCommand & MGDEMO_LOOP))
	{
		if(nCommand & MGDEMO_MAKE_HD_MODULE)
		{
			/* ハードディスク対応用のデータ作成 */
			/* stream */
			sprintf(szCommand, "cd list; stream demoscr/%s.scr -d ./stream.def -o ../cdrom.img/stage/../demo.dat -t ../scn/demo.lst;", lpszTarget);
			if(system(szCommand) != 0)
				return 0;

			/* コンパイルするステージを直接指定 */
			sprintf(szCommand, "%s.gcl", lpszTarget);
			setenv("DEMO_SCENARIOS", szCommand, 1);

			/* make scn */
			if(system(lppMakeCommand[RUN_SCN]) != 0)
				return 0;

//			if((nCommand & MGDEMO_CUT_CHECK) == 0)
//			{
				/* make stage */
				if(system(lppMakeCommand[RUN_STAGE]) != 0)
					return 0;

				/* make link */
				sprintf(szCommand, "%s\"init %s %s\"%s", lppMakeHDModule[0],
					lpszResident, lpszTarget, lppMakeHDModule[1]);
				if(system(szCommand) != 0)
					return 0;
//			}

			/* デモ用のディレクトリがあれば削除 */
			sprintf(szCommand, "%s/stage/%s", lpszCDRomImgDirectory, DEMO_HD_STAGE);
			printf("Delete %s\n", szCommand);
			if(CheckDirectory(szCommand, 0) == 1)
			{
				sprintf(szCommand, "rm -rf %s/stage/%s", lpszCDRomImgDirectory, DEMO_HD_STAGE);
				if(system(szCommand) != 0)
					return 0;
			}
#if 0
			/* ディレクトリを複製 */
			sprintf(szCommand, "cp -ur %s/stage/%s %s/stage/%s",
				lpszCDRomImgDirectory, lpszTarget, lpszCDRomImgDirectory, DEMO_HD_STAGE);
			printf("Copy %s\n", szCommand);
			if(system(szCommand) != 0)
				return 0;
#else
			/* リンクファイルがあったら削除 */
			sprintf(szCommand, "rm %s/stage/%s", lpszCDRomImgDirectory, DEMO_HD_STAGE);
			system(szCommand);

			/* シンボリックリンクを作成 */
			sprintf(szCommand, "cd cdrom.img/stage; ln -s %s %s", lpszTarget, DEMO_HD_STAGE);
//			printf("%s\n", szCommand);
			system(szCommand);
#endif

			/* ee-strip */
			sprintf(szCommand, "%s %s/mgs2.elf -o %s/hdboot.elf;", lppMakeHDModule[2],
				lpszCDRomImgDirectory, lpszCDRomImgDirectory);
			if(system(szCommand) != 0)
				return 0;

			/* mkcdimg用のファイルを調べる */
			if(CheckFile("source/demo_hd_stage.lst", 0) == 1)
			{
				/* すでにあるので削除 */
				if(system("rm source/demo_hd_stage.lst") != 0)
					return 0;
			}

			/* mkcdimg用のファイルを作成 */
			sprintf(szCommand, "echo \" init %s %s\" > source/demo_hd_stage.lst", lpszResident, DEMO_HD_STAGE);
			if(system(szCommand) != 0)
				return 0;

			/* mkcdimg */
			sprintf(szCommand, "%s -l demo_hd_stage.lst -o %s/stage.dat -s %s/stage -z;",
				lppMakeHDModule[3], lpszCDRomImgDirectory, lpszCDRomImgDirectory);
			if(system(szCommand) != 0)
				return 0;

			/* hdinst.cnf のチェック、及び作成 */
			if(nCommand & MGDEMO_HD_FORMAT)
			{
				sprintf(szCommand, "cd cdrom.img; cvs update -dPA hdinst.demo");
				system(szCommand);
				sprintf(szCommand, "cd cdrom.img; rm hdinst.cnf");
				system(szCommand);
				sprintf(szCommand, "cd cdrom.img; mv hdinst.demo hdinst.cnf");
				system(szCommand);
			}
			else
			{
				sprintf(szCommand, "cd cdrom.img; cvs update -dPA hdinst_no_form.demo");
				system(szCommand);
				sprintf(szCommand, "cd cdrom.img; rm hdinst.cnf");
				system(szCommand);
				sprintf(szCommand, "cd cdrom.img; mv hdinst_no_form.demo hdinst.cnf");
				system(szCommand);
			}

			/* hdboot.cnf のチェック、及び作成 */
			if(CheckFile("cdrom.img/hdboot.cnf", 0) == 0)
			{
				/* 無いので作成 */
				sprintf(szCommand, "cd cdrom.img; cvs update -dPA hdboot.cnf");
				if(system(szCommand) != 0)
					return 0;
			}
		}
		else
		{
				/* stream */
				sprintf(szCommand, "cd list; stream demoscr/%s.scr -d ./stream.def -o ../cdrom.img/stage/../demo.dat -t ../scn/demo.lst;", lpszTarget);
				if(system(szCommand) != 0)
					return 0;

				/* コンパイルするステージを直接指定 */
				sprintf(szCommand, "%s.gcl", lpszTarget);
				setenv("DEMO_SCENARIOS", szCommand, 1);

				/* make scn */
				if(system(lppMakeCommand[RUN_SCN]) != 0)
					return 0;

//				if((nCommand & MGDEMO_CUT_CHECK) == 0)
//				{
					/* make stage */
					if(system(lppMakeCommand[RUN_STAGE]) != 0)
						return 0;

					/* make link */
					sprintf(szCommand, "%s\"init %s %s\"%s", lppMakeLinkOneStage[0],
						lpszResident, lpszTarget, lppMakeLinkOneStage[1]);
					if(system(szCommand) != 0)
						return 0;
//				}

				/* make go */
				if(system(lppMakeCommand[RUN_GO]) != 0)
					return 0;
			}
	}

	return 1;
}
