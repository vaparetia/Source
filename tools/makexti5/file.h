#ifndef _FILE_H_
#define _FILE_H_

extern FILE *KsFileOpen(char *fname, char *mode);
extern void KsChangeFileName(char *pname, char *pcname, char *pext);
extern u_long KsFileSize(FILE *rfp);
extern long KsFileToBuf(FILE *fp, BYTE *pbuf);
extern short read_listfile(char *filename,char **pstr2,char *pwbuf);
extern void ChangeFileNameUpper(char *pname, char *pname2);
extern void DeleteKakutyousi(char *pname, char *pname2);
extern short get_path(char *pname, char *pname2);
extern char *get_file(char *pname,char *pname2);
extern char *cat_string(char *s1,char *s2, char *s3);
extern void print_ftell(FILE *nfp);
extern long fwrite2work(const void *buf,int size,int n,void *tbuf);
//extern short rect2file(RECT2 *prect,u_char *filename);
//extern short rect2wfp(RECT2 *prect,FILE *wfp);
#endif