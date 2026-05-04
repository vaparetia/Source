#pragma once

#ifndef __XTEXTSCN_H__
#define __XTEXTSCN_H__

/* 説明分テキストを展開する */
int MENU_CreateTextTexture( void *work, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message );
int MENU_CreateTextTextureNoConvert_VariableResult( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message, int getResultType );
int MENU_CreateTextTextureNoConvert( void *work, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message );
int MENU_CreateTextTextureNoConvertGetLines( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message );
int MENU_CreateTextTextureGetLines( void *work_ptr, int x, int y, int width, int height, int pitch, int space, int max_lines, char *message );

#endif // __XTEXTSCN_H_