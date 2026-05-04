#ifndef _cdc_load_h_
#define _cdc_load_h_

void   CDC_InitLoadingModule(void);
int    CDC_CalcFileParamByEntry(unsigned int entry,
				unsigned int *offset, unsigned int *size);

void * CDC_LoadLogicalDirectory(int entry);
int    CDC_SyncCodecDataLoad(void);
void   CDC_InitLoadData(void);
void   CDC_ReleaseLogicalDirectory(void);
void * CDC_GetFileEntry(int strcode, int suffix);
void * CDC_LoadTRI(int name);
void * CDC_LoadEvmDef(int name);

#endif /* _cdc_load_h_ */
