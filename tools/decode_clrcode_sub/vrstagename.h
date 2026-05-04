/*

  2002/08/21 M.Kobayashi
  $Id: vrstagename.h,v 1.2 2002/08/21 09:28:40 usr03700 Exp $
  
 */

extern int GetVrStageName( int id, 
						   int platform, int region,
						   char* pplayer, char* pmission, char* pmode, char* psubmode, char* plevel );
// VR のステージの名称をIDから取得します。


#define VR_STAGENAME_MAX	64



