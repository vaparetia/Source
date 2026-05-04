/*
	hair_called.h
		髪の毛(長髪タイプ)シミュレーション

	2000/06/02 K.Kano
	$Id: hair_called.h,v 1.1.1.3 2002/11/19 11:43:11 Yoshizawa1 Exp $
*/


#ifndef _hair_called_h_
#define _hair_called_h_


void *NewHairModel_called(int model_name,int sample_num,
						  OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
						  float oval_param,int collision_flag,unsigned char *collision_objs,
						  int visible_flag,int light_flag,int boundmodel_name);

void *NewEvmHairModel_called(int model_name,int sample_num,
							 OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
							 float oval_param,int collision_flag,unsigned char *collision_objs,
							 int visible_flag,int light_flag,int boundmodel_name);

void *NewEvmHairModel_called2(int name,int model_name,int sample_num,
							  OBJECT *target,unsigned char *objnum,int n_objnum,
							  FVECTOR *x,SVECTOR *r,
							  float oval_param,int collision_flag,unsigned char *collision_objs,
							  int visible_flag,int light_flag,int boundmodel_name);

void *NewHairModel_Demo(int name,int model_name,int sample_num,
                        OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
                        float oval_param,int collision_flag,
                        int visible_flag,int light_flag,int boundmodel_name);

void *NewEvmHairModel_Demo(int name,int model_name,int sample_num,
                           OBJECT *target,int objnum,FVECTOR *x,SVECTOR *r,
                           float oval_param,int collision_flag,
                           int visible_flag,int light_flag,int boundmodel_name);

/*
  サンプル番号 0-3まで
   0 ... フォーチュンの裾揺れ用
   1 ... フォーチュンの右袖揺れ用
   2 ... フォーチュンの左袖揺れ用
   3 ... フォーチュンの髪の毛揺れ用
 */

void *NewEvmHairModel_Demo2(int name,int model_name,int sample_num,
							OBJECT *target,FVECTOR *x,SVECTOR *r,
							float oval_param,int visible_flag,int light_flag,int boundmodel_name);

void *NewHairModel_Wireless(int name,DG_DEF *kms,DG_DEF *bound,int sample_num,
							OBJECT *target,int light_flag,int chanl_num);

void *NewEvmHairModel_Wireless(int name,EVM_DEF *evm,DG_DEF *bound,int sample_num,
							   OBJECT *target,int light_flag,int chanl_num,int draw_flag,
							   int no_wind_flag);

#endif
