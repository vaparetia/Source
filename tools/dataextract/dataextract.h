#ifndef __DATAEXTRACT_H__
#define __DATAEXTRACT_H__

#include <string>

enum EUsageException { kUsageException };
enum EAbortException { kAbortException };

extern std::string const GetExtensionForTag( unsigned const tag_id );
extern void add_file_to_archive_xml( FILE * fp, const char * const filePath, const char * const archivePath );

#endif