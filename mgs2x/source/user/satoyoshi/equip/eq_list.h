/********************************************************************************/
/*	structure      								*/
/*	$Id: eq_list.h,v 1.1.1.3 2002/11/19 11:48:18 Yoshizawa1 Exp $									*/
/********************************************************************************/

#ifndef __eqp_eq_list_h__
#define __eqp_eq_list_h__

typedef struct
{
    char	*m_name ;
    short	joint_id;
    FVECTOR	shift;
    SVECTOR	rot;
}EQ_DATA;


static EQ_DATA ussoldier_data[] = {
{ "obj_cap",		12,{   0.0F,    0.0F,    0.0F, 0.0F},	{  0,   0,   0, 0} },//帽子
{ "obj_glass",		12,{   0.0F,    0.0F,    0.0F, 0.0F},	{  0,   0,   0, 0} },//サングラス
{ "obj_helmet_def_s",	12,{   0.0F,    0.0F,    0.0F, 0.0F},	{  0,   0,   0, 0} },//ヘルメット
{ "obj_headphone_def",	12,{   0.0F,    0.0F,    0.0F, 0.0F},	{  0,   0,   0, 0} },//ヘッドフォン
{ "m4a_nm",		2, {-190.0F, -265.0F,  -60.0F, 0.0F},	{2048,  0,   0, 0} },//Ｍ４肩
{ "m4a_nm",		6, {   0.0F,    0.0F,    0.0F, 0.0F},	{  0,   0,   0, 0} },//Ｍ４右手
{ "m92",		6, {   0.0F,    0.0F,    0.0F, 0.0F},	{  0,   0,   0, 0} },//Ｍ９２右手
    };

static EQ_DATA citizen_male_data[] = {
    { "cit_male_hair01",  HUMAN21_ATAMA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//髪の毛 茶色 0
    { "cit_male_hair02",  HUMAN21_ATAMA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//髪の毛 金色 1
    { "cit_male_hair04",  HUMAN21_ATAMA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//髪の毛 白色 2
    { "cit_male_hair05",  HUMAN21_ATAMA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//髪の毛 ハゲ 3

    { "cit_male_shirtbk", HUMAN21_MUNE ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//シャツ黒 4
    { "cit_male_shirtbl", HUMAN21_MUNE ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//シャツ青 5
    { "cit_male_tieye"  , HUMAN21_MUNE ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//ネクタイ黄色 6
    { "cit_male_tiebr"  , HUMAN21_MUNE ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//ネクタイ茶色 7

    { "cit_male_glass01", HUMAN21_ATAMA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//眼鏡 8

    { "cit_male_bag02" , HUMAN21_MIGI_TE  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//バッグ 右手 9
    { "cit_male_bag01" , HUMAN21_HIDARI_TE,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//バッグ 左手 10
    { "cit_male_bag04" , HUMAN21_HIDARI_KATA  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//バッグ 右肩
    { "cit_male_bag03" , HUMAN21_MIGI_KATA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//バッグ 左肩
    { "cit_male_bag06" , HUMAN21_HIDARI_KATA  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//バッグ 右肩
    { "cit_male_bag05" , HUMAN21_MIGI_KATA,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//バッグ 左肩

} ;


static EQ_DATA citizen_female_data[] = 
{
    { "cit_female_hair_a_1" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_a_2" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_a_3" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_a_4" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_a_5" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り

    { "cit_female_hair_b_1" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_b_2" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_b_3" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_b_4" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り
    { "cit_female_hair_b_5" , HUMAN21_ATAMA ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//頭部飾り

    { "cit_female_shirts_1" , HUMAN21_MUNE  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//女性用ブラウス
    { "cit_female_shirts_2" , HUMAN21_MUNE  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//女性用ブラウス
    { "cit_female_shirts_3" , HUMAN21_MUNE  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//女性用ブラウス
    { "cit_female_shirts_4" , HUMAN21_MUNE  ,{ 0.0f, 0.0f, 0.0f, 1.0f}, { 0, 0, 0, 0} },//女性用ブラウス
} ;


/* ライデン用のワンピース銃 */
static EQ_DATA arms_equip_data[] = 
{
    { "m92_rai_1piece" , HUMAN21_MIGI_TE ,{0.0f,0.0f,0.0f,1.0f}, {0,0,0,0} },
} ;

static int equip_n_list[] = 
{
    sizeof(ussoldier_data)/sizeof(EQ_DATA),
    sizeof(citizen_male_data)/sizeof(EQ_DATA),
    sizeof(citizen_female_data)/sizeof(EQ_DATA),
    sizeof(arms_equip_data)/sizeof(EQ_DATA),
} ;

static EQ_DATA *equip_list[] = 
{
    ussoldier_data,
    citizen_male_data,
    citizen_female_data, 
    arms_equip_data,
} ;

#endif
