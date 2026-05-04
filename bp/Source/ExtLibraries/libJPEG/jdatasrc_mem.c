/*
 *
 * jmemdatasrc.c
 *
 * AndyO
 * 
 * Sets up a read from memory src_manager
 */

#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"


METHODDEF(void) my_jpeg_decompress_error_exit(j_common_ptr cinfo) 
{ 
    //throw RuntimeException(__FILE__, __LINE__, "Jpeg decompression error!"); 
} 


METHODDEF(void) my_jpeg_init_source(j_decompress_ptr cinfo) 
{ 


} 


METHODDEF(boolean) my_jpeg_fill_input_buffer(j_decompress_ptr cinfo) 
{ 
    ERREXIT(cinfo, 0); 
    return FALSE; 


} 


METHODDEF(void) my_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes) 
{ 
    if(num_bytes > (long) (cinfo)->src->bytes_in_buffer) { 
        ERREXIT(cinfo, 0); 
    } 
    (cinfo)->src->next_input_byte += num_bytes; 
    (cinfo)->src->bytes_in_buffer -= num_bytes; 


} 


METHODDEF(void) my_jpeg_term_source(j_decompress_ptr cinfo) 
{
}


GLOBAL(void) jpeg_init_memory_stream_source(struct jpeg_source_mgr * src_mgr, void * pData, long dataSize) 
{ 

    // Setup a minimal RAM source manager. 
    src_mgr->init_source       = my_jpeg_init_source; 
    src_mgr->fill_input_buffer = my_jpeg_fill_input_buffer; 
    src_mgr->skip_input_data   = my_jpeg_skip_input_data; 
    src_mgr->resync_to_restart = jpeg_resync_to_restart; 
    src_mgr->term_source       = my_jpeg_term_source; 
    src_mgr->bytes_in_buffer   = dataSize; 
    src_mgr->next_input_byte   = (JOCTET *) pData; 
} 

