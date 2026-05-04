#ifndef _util_h_
#define _util_h_

char * utilDupStr(char *str);
char * utilReadLine(FILE *rfp);
char * utilStrFips(char *str);
char * utilPathAppend(char *path, char *fname);
void   utilSprit(char **class, char **value, char *line);
void * utilAddBuf(void *buf, size_t siz, size_t nums);
int utilCheckChars(int c, char *list);
int  utilStr2Int(int *ret, char *str);

#endif /* _util_h_ */
