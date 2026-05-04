#ifndef _txresource_h_
#define _txresource_h_


typedef struct txEntry {
  char * tag;
  char * value;
} txEntry;

typedef struct txRes {
  int nums;
  txEntry * entry;
} txRes;


txRes * txrGetResource(FILE *rfp);
void txrRelease(txRes * res);
char * txrRef(txRes * res, char * tag);

#endif /* _txresource_h_ */
