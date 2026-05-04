/*
	resource.h
	常駐リソース管理

	2000/10/14	K.Uehara
	$Id: resource.h,v 1.1.1.3 2002/11/19 11:41:55 Yoshizawa1 Exp $
*/

#ifdef __LIBGCL__H__
void GM_SetResourceInfo( GCL_STRING_RESOURCE *res );
void GM_SetResourceId( int ref_id, int res_id );
#endif
void *GM_GetResource( int ref_id, int offset );

