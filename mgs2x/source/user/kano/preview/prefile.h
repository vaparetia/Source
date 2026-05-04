/*
	prefile.h
		data.cnfから、現在読み込まれているファイルの名前を取得。
		必要に応じて分類も行なう。

	1999/12/13 K.Kano
	$Id: prefile.h,v 1.1.1.3 2002/11/19 11:43:29 Yoshizawa1 Exp $
*/


#ifndef _prefile_h_
#define _prefile_h_


typedef struct {
    char *data;
    int data_size;

    char **filenames;
    int filenames_size;

    char **human_filenames;
    char *human_format_flags;
    int human_filenames_size;
    char **mar_filenames;
    int mar_filenames_size;

    char **object_filenames;
    int object_filenames_size;

    char **faceanim_filenames;
    int faceanim_filenames_size;

    char **tex_filenames;
    int tex_filenames_size;

    char **stagelight_filenames;
    int stagelight_filenames_size;

    char **objchange_filenames;
    int objchange_filenames_size;

    char **vanime_filenames;
    int vanime_filenames_size;

  char ** layout_filenames;
  int     layout_filenames_size;

	char **fobj_filenames;
	char **fobj_mar_filenames;
	char *fobj_format_flags;
	int fobj_filenames_size;
} PreviewFile_Param;


#ifdef _WORK_DECLARED_
PreviewFile_Param PreviewFile;
#else
extern PreviewFile_Param PreviewFile;
#endif

int PreviewSaveBinFile(char *filename,void *buf,int size);
int PreviewLoadBinFile(char *filename,void *buf,int size);

int LoadDataConf(void);
void FreePreviewFile(void);


#endif
