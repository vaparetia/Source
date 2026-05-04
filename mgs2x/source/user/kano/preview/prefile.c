/*
	prefile.c
		data.cnfから、現在読み込まれているファイルの名前を取得。
		必要に応じて分類も行なう。

	1999/12/13 K.Kano
	$Id: prefile.c,v 1.1.1.3 2002/11/19 11:43:29 Yoshizawa1 Exp $
*/


#include "preview_def.h"


/* ---------------------------------------------------------------- */

#define sceOpen( a, b )		pcOpen( a, b )
#define sceRead( a, b, c )	pcRead( a, b, c )
#define sceWrite( a, b, c ) pcWrite( a, b, c )
#define sceClose( a )		pcClose( a )
#define sceLseek( a, b, c )	pcLseek( a, b, c )

int PreviewSaveBinFile(char *filename,void *buf,int size)
{
    int wfd;
	int ans=1;

    /* ファイルの書き込みオープン */
    wfd = sceOpen(filename,SCE_WRONLY|SCE_TRUNC|SCE_CREAT);
    if (wfd < 0 ) {
		/* エラー処理 */
		printf("Can't open %s\n",filename);
		return 0;
    }

    if(sceWrite(wfd,(unsigned char *)buf,size)<0){
		printf("Can't write %s\n",filename);
		ans=0;
	}
    sceClose(wfd);

	return ans;
}

int PreviewLoadBinFile(char *filename,void *buf,int size)
{
    int rfd;
	int ans=1;

    /* ファイルの読み込みオープン */
    rfd = sceOpen(filename,SCE_RDONLY);
    if (rfd < 0 ) {
		/* エラー処理 */
		printf("Can't open  %s\n",filename);
		return 0;
    }

    if(sceRead(rfd,(unsigned char *)buf,size)<0){
		printf("Can't read %s\n",filename);
		ans=0;
	}
    sceClose(rfd);

	return ans;
}


/* ---------------------------------------------------------------- */


#if 0

typedef struct {
    int handle;
    int mode;
} PC_FILE;

static PC_FILE *PCFileOpen(char *filename,int mode)
{
    PC_FILE *fp;

    if((fp=(PC_FILE *)GV_Malloc(sizeof(PC_FILE)))==NULL) return NULL;

    fp->handle=sceOpen(filename,mode);
    fp->mode=mode;

    if(fp->handle<0){
		GV_Free(fp);
		return NULL;
    }

    return fp;
}

static void PCFileClose(PC_FILE *fp)
{
    if(fp==NULL) return;
    sceClose(fp->handle);
    GV_Free(fp);
}

static void PCFilePutc(int c,PC_FILE *fp)
{
    char tmp=c;
    sceWrite(fp->handle,&tmp,sizeof(char));
}

static int PCFileGetc(PC_FILE *fp)
{
    unsigned char c;
    if(sceRead(fp->handle,&c,sizeof(char))<0) return -1;
    return c;
}

#endif


/* ---------------------------------------------------------------- */


static void InitPreviewFile(void)
{
    /* ワークの初期化 */
    PreviewFile.data=NULL;
    PreviewFile.data_size=0;

    PreviewFile.filenames=NULL;
    PreviewFile.filenames_size=0;

    PreviewFile.human_filenames=NULL;
    PreviewFile.human_format_flags=NULL;
    PreviewFile.human_filenames_size=0;

    PreviewFile.object_filenames=NULL;
    PreviewFile.object_filenames_size=0;

    PreviewFile.mar_filenames=NULL;
    PreviewFile.mar_filenames_size=0;

    PreviewFile.faceanim_filenames=NULL;
    PreviewFile.faceanim_filenames_size=0;

    PreviewFile.tex_filenames=NULL;
    PreviewFile.tex_filenames_size=0;

    PreviewFile.stagelight_filenames=NULL;
    PreviewFile.stagelight_filenames_size=0;

    PreviewFile.objchange_filenames=NULL;
    PreviewFile.objchange_filenames_size=0;

    PreviewFile.layout_filenames = NULL;
    PreviewFile.layout_filenames_size = 0;

    PreviewFile.vanime_filenames=NULL;
    PreviewFile.vanime_filenames_size=0;

	PreviewFile.fobj_filenames=NULL;
	PreviewFile.fobj_mar_filenames=NULL;
	PreviewFile.fobj_format_flags=NULL;
	PreviewFile.fobj_filenames_size=0;
}

static int GetPreviewFileFilenames(void)
{
    int rfd,len,i;
    char *cp;
    int	count;

    /* ファイルの読み込みオープン */
    rfd = sceOpen("host0:./stage/preview/data.cnf", SCE_RDONLY);
    if (rfd < 0 ) {
		/* エラー処理 */
		printf("Can't open data.cnf\n");
		return 0;
    }

    /* ファイルサイズの確認 */
    len = sceLseek(rfd, 0, SCE_SEEK_END); /* サイズを知って*/
    sceLseek(rfd, 0, SCE_SEEK_SET); /* offset を戻しておく */

    PreviewFile.data=(char *)GV_Malloc( len+1 );
    if(PreviewFile.data==NULL){
		printf("Error : Memory Overflow.\n");
		sceClose(rfd);
		return 0;
    }
    PreviewFile.data_size=len;

    sceRead(rfd,PreviewFile.data,len);

    /* ファイルのクローズ */
    sceClose(rfd);

    /* ファイル名の数の確認 */
    cp=PreviewFile.data;
    i=0;
    count=0;
    while(i<len){
		if(*cp!='.') count++;

		while(i<len && *cp!='\n'){
			i++,cp++;
		}
		*cp='\0';

		i++;
		cp++;
    }

    if(count==0) return 0;


    /* ファイル名の切り出し */
    if((PreviewFile.filenames=(char **)GV_Malloc(sizeof(char *)*count))==NULL){
		printf("Error : Memory Overflow.\n");
		return 0;
    }
    PreviewFile.filenames_size=count;

    cp=PreviewFile.data;
    i=0;
    while(i<count){
		if(*cp!='.'){
			*(PreviewFile.filenames+i)=cp;
			i++;
		}
		while(*cp!='\0') cp++;
		cp++;
    }

    return 1;
}

static void RemoveSuffix(char *a)
{
    int i;

    i=0;
    while(*(a+i)!='\0') i++;
    while(i>=0 && *(a+i)!='.') i--;
    if(i<=0) return;
    *(a+i)='\0';
}

static void RemoveSuffixFromFilenames(char **str,int size)
{
    int i;
    for(i=0;i<size;i++){
		if(*(str+i)!=NULL) RemoveSuffix(*(str+i));
	}
}

static int CheckSuffix(char *filename,char *suffix)
{
    int i;

    i=0;
    while(*(filename+i)!='\0') i++;
    while(i>=0 && *(filename+i)!='.') i--;

    if(i<0) return 0;

    filename+=i+1;

    while(*filename!='\0' || *suffix!='\0'){
		if(*filename!=*suffix) return 0;
		filename++;
		suffix++;
    }
    return 1;
}

static int CheckHead(char *str,char *head)
{
    while(*head!='\0'){
		if(*str!=*head) return 0;
		str++;
		head++;
    }
    return 1;
}

static int CheckPurefilename(char *str,char *fname)
{
    while(*fname!='\0'){
		if(*str!=*fname) return 0;
		str++;
		fname++;
    }
	if(*str!='.') return 0;
    return 1;
}

static int GetPreviewFileTypes(int *check_list,char **head,int head_size,
							   char **suffix,int suffix_size,
							   char ***filelist,int *listsize)
{
    int i,j,count;

    i=0;
    count=0;
    for(i=0,count=0;i<PreviewFile.filenames_size;i++){
		if(!*(check_list+i)){
			if(suffix!=NULL){
				if(suffix_size==1){
					if(!CheckSuffix(*(PreviewFile.filenames+i),(char *)suffix)) goto forbreak;
				}
				else{
					for(j=0;j<suffix_size;j++){
						if(CheckSuffix(*(PreviewFile.filenames+i),*(suffix+j))) goto success1;
					}
					goto forbreak;
				}
success1:
			}
			if(head!=NULL){
				if(head_size==1){
					if(!CheckHead(*(PreviewFile.filenames+i),(char *)head)) goto forbreak;
				}
				else{
					for(j=0;j<head_size;j++){
						if(CheckHead(*(PreviewFile.filenames+i),*(head+j))) goto success2;
					}
					goto forbreak;
				}
success2:
			}
			(*(check_list+i))++;
			count++;
		}

forbreak:

    }
    if(count==0) return 1;

    if(filelist!=NULL){
		if((*filelist=(char **)GV_Malloc(sizeof(char *)*count))==NULL){
			printf("Error : Memory Overflow.\n");
			return 0;
		}
		*listsize=count;

		i=0; j=0;
		while(i<count){
			if(*(check_list+j)==1){
				*(*filelist+i)=*(PreviewFile.filenames+j);
				(*(check_list+j))++;
				i++;
			}
			j++;
		}
    }
    else{
		i=0; j=0;
		while(i<count){
			if(*(check_list+j)==1){
				(*(check_list+j))++;
				i++;
			}
			j++;
		}
    }

    return 1;
}

static int GetPreviewFileType(int *check_list,char *head,char *suffix,
							  char ***filelist,int *listsize)
{
	return GetPreviewFileTypes(check_list,(char **)head,1,(char **)suffix,1,
							   filelist,listsize);
}

static int GetPreviewFileByName(int *check_list,char **check_filenames,int check_size,
								char *suffix,char ***filelist)
{
	int i,j;

	if((*filelist=GV_Malloc(sizeof(char *)*check_size))==NULL) return 0;

	for(j=0;j<check_size;j++){
		*(*filelist+j)=NULL;

		if(*(check_filenames+j)==NULL) continue;

		for(i=0;i<PreviewFile.filenames_size;i++){
			if(suffix!=NULL){
				if(!CheckSuffix(*(PreviewFile.filenames+i),suffix)) continue;
			}
			if(!CheckPurefilename(*(PreviewFile.filenames+i),*(check_filenames+j))){
				continue;
			}
			*(*filelist+j)=*(PreviewFile.filenames+i);
			(*(check_list+j))+=2;
			break;
		}
	}

	return 1;
}

static int GetPreviewFileHumanFilenames(int *check_list)
{
    int i;

    GetPreviewFileType(check_list,"human","cv2",NULL,NULL);

    if(!GetPreviewFileType(check_list,"human",NULL,
						   &(PreviewFile.human_filenames),
						   &(PreviewFile.human_filenames_size))){

		return 0;
    }

    if((PreviewFile.human_format_flags
		=(char *)GV_Malloc(sizeof(char)*PreviewFile.human_filenames_size))==NULL) return 0;

    for(i=0;i<PreviewFile.human_filenames_size;i++){
		if(CheckSuffix(*(PreviewFile.human_filenames+i),"evm")){
			*(PreviewFile.human_format_flags+i)=1;
		}
		else{
			*(PreviewFile.human_format_flags+i)=0;
		}
    }

    RemoveSuffixFromFilenames(PreviewFile.human_filenames,PreviewFile.human_filenames_size);

    if(!GetPreviewFileType(check_list,"motion","mar",
						   &(PreviewFile.mar_filenames),&(PreviewFile.mar_filenames_size))){

		return 0;
	}

    RemoveSuffixFromFilenames(PreviewFile.mar_filenames,PreviewFile.mar_filenames_size);

    return 1;
}

static int GetPreviewFileFaceanimFilenames(int *check_list)
{
    int ans;
    ans=GetPreviewFileType(check_list,NULL,"rmt",
						   &(PreviewFile.faceanim_filenames),&(PreviewFile.faceanim_filenames_size));
    RemoveSuffixFromFilenames(PreviewFile.faceanim_filenames,PreviewFile.faceanim_filenames_size);
    return ans;
}

static int GetPreviewFileTexFilenames(int *check_list)
{
    int ans;
    ans=GetPreviewFileType(check_list,NULL,"tri",
						   &(PreviewFile.tex_filenames),&(PreviewFile.tex_filenames_size));
    RemoveSuffixFromFilenames(PreviewFile.tex_filenames,PreviewFile.tex_filenames_size);
    return ans;
}

static int GetPreviewFileStagelightFilenames(int *check_list)
{
    int ans;
    ans=GetPreviewFileType(check_list,NULL,"lt2",
						   &(PreviewFile.stagelight_filenames),
						   &(PreviewFile.stagelight_filenames_size));
    RemoveSuffixFromFilenames(PreviewFile.stagelight_filenames,
							  PreviewFile.stagelight_filenames_size);
    return ans;
}

static int GetPreviewFileObjchangeFilenames(int *check_list)
{
    int ans;
    GetPreviewFileType(check_list,"objch","cv2",NULL,NULL);
    ans=GetPreviewFileType(check_list,"objch","kms",
						   &(PreviewFile.objchange_filenames),
						   &(PreviewFile.objchange_filenames_size));
    RemoveSuffixFromFilenames(PreviewFile.objchange_filenames,
							  PreviewFile.objchange_filenames_size);
    return ans;
}

static int GetPreviewFileLayoutFilenames(int * check_list)
{
  int ans;

  ans = GetPreviewFileType(check_list, NULL, "o2d",
			   &(PreviewFile.layout_filenames),
			   &(PreviewFile.layout_filenames_size));
  RemoveSuffixFromFilenames(PreviewFile.layout_filenames,
			    PreviewFile.layout_filenames_size);
  return ans;
}


static int GetPreviewFileVanimeFilenames(int *check_list)
{
    int ans;
    GetPreviewFileType(check_list,"vanim","kms",NULL,NULL);
    ans=GetPreviewFileType(check_list,"vanim","cv2",
						   &(PreviewFile.vanime_filenames),
						   &(PreviewFile.vanime_filenames_size));
    RemoveSuffixFromFilenames(PreviewFile.vanime_filenames,
							  PreviewFile.vanime_filenames_size);
    return ans;
}

static int GetPreviewFileObjectFilenames(int *check_list)
{
    int ans;
    ans=GetPreviewFileType(check_list,NULL,"kms",
						   &(PreviewFile.object_filenames),
						   &(PreviewFile.object_filenames_size));
    RemoveSuffixFromFilenames(PreviewFile.object_filenames,
							  PreviewFile.object_filenames_size);
    return ans;
}

static int GetPreviewFileFobjFilenames(int *check_list)
{
	static const char *suffix[]={
		"kms",
		"evm",
	};
	int i;

    GetPreviewFileType(check_list,"fobj","cv2",NULL,NULL);

	if(!GetPreviewFileTypes(check_list,(char **)"fobj",1,(char **)suffix,2,
							&(PreviewFile.fobj_filenames),
							&(PreviewFile.fobj_filenames_size))){

		return 0;
	}

    if((PreviewFile.fobj_format_flags
		=(char *)GV_Malloc(sizeof(char)*PreviewFile.fobj_filenames_size))==NULL) return 0;

    for(i=0;i<PreviewFile.fobj_filenames_size;i++){
		if(CheckSuffix(*(PreviewFile.fobj_filenames+i),"evm")){
			*(PreviewFile.fobj_format_flags+i)=1;
		}
		else{
			*(PreviewFile.fobj_format_flags+i)=0;
		}
    }

    RemoveSuffixFromFilenames(PreviewFile.fobj_filenames,
							  PreviewFile.fobj_filenames_size);

	if(!GetPreviewFileByName(check_list,
							 PreviewFile.fobj_filenames,PreviewFile.fobj_filenames_size,
							 "mar",
							 &(PreviewFile.fobj_mar_filenames))){

		return 0;
	}

    RemoveSuffixFromFilenames(PreviewFile.fobj_mar_filenames,
							  PreviewFile.fobj_filenames_size);
    GetPreviewFileType(check_list,"fobj","mar",NULL,NULL);

	return 1;
}

int LoadDataConf(void)
{
    int *check_list;

    InitPreviewFile();

    if(!GetPreviewFileFilenames()) return 0;

    if((check_list=GV_Malloc(sizeof(int)*PreviewFile.filenames_size))==NULL) return 0;
    GV_ZeroMemory(check_list,sizeof(int)*PreviewFile.filenames_size);

    GetPreviewFileTexFilenames(check_list);

    GetPreviewFileHumanFilenames(check_list);
    GetPreviewFileStagelightFilenames(check_list);
    GetPreviewFileObjchangeFilenames(check_list);
    GetPreviewFileVanimeFilenames(check_list);
    GetPreviewFileLayoutFilenames(check_list);
	GetPreviewFileFobjFilenames(check_list);

    /* 最後に余ったものが、Object用 */
    GetPreviewFileObjectFilenames(check_list);
    GetPreviewFileFaceanimFilenames(check_list);

    GV_Free(check_list);

    return 1;
}

void FreePreviewFile(void)
{
    if(PreviewFile.data!=NULL) GV_Free(PreviewFile.data);
    if(PreviewFile.filenames!=NULL) GV_Free(PreviewFile.filenames);

    if(PreviewFile.human_filenames!=NULL) GV_Free(PreviewFile.human_filenames);
    if(PreviewFile.human_format_flags!=NULL) GV_Free(PreviewFile.human_format_flags);
    if(PreviewFile.object_filenames!=NULL) GV_Free(PreviewFile.object_filenames);
    if(PreviewFile.mar_filenames!=NULL) GV_Free(PreviewFile.mar_filenames);
    if(PreviewFile.faceanim_filenames!=NULL) GV_Free(PreviewFile.faceanim_filenames);
    if(PreviewFile.tex_filenames!=NULL) GV_Free(PreviewFile.tex_filenames);
    if(PreviewFile.stagelight_filenames!=NULL) GV_Free(PreviewFile.stagelight_filenames);
    if(PreviewFile.objchange_filenames!=NULL) GV_Free(PreviewFile.objchange_filenames);
    if(PreviewFile.vanime_filenames!=NULL) GV_Free(PreviewFile.vanime_filenames);

    if(PreviewFile.layout_filenames != NULL) GV_Free(PreviewFile.layout_filenames);
    if(PreviewFile.fobj_filenames!=NULL) GV_Free(PreviewFile.fobj_filenames);
    if(PreviewFile.fobj_mar_filenames!=NULL) GV_Free(PreviewFile.fobj_mar_filenames);
    if(PreviewFile.fobj_format_flags!=NULL) GV_Free(PreviewFile.fobj_format_flags);
}
