# Microsoft Developer Studio Project File - Name="okajima" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** 編集しないでください **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=okajima - Xbox Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "okajima.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "okajima.mak" CFG="okajima - Xbox Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "okajima - Xbox Release" ("Xbox Static Library" 用)
!MESSAGE "okajima - Xbox Debug" ("Xbox Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/mgs2x/okajima", IRBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe

!IF  "$(CFG)" == "okajima - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "okajima - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "okajima___Xbox_Debug"
# PROP BASE Intermediate_Dir "okajima___Xbox_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\include" /I "..\..\system\libgv" /I "..\..\system\libdg" /I "..\..\system\libgcl" /I "..\..\system\libfs" /I "..\..\system\libutl" /I "..\..\system\libhzx" /I "..\..\game" /I "..\..\system\libmt" /D "WIN32" /D "_XBOX" /D "_DEBUG" /D "DEBUG" /D "DEBUG_MODE" /D "NTSC" /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "okajima - Xbox Release"
# Name "okajima - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "effect"

# PROP Default_Filter ""
# End Group
# Begin Group "effect2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\effect2\auto_splush.c
# End Source File
# Begin Source File

SOURCE=.\effect2\blood_strip.c
# End Source File
# Begin Source File

SOURCE=.\effect2\blood_strip_parts.c
# End Source File
# Begin Source File

SOURCE=.\effect2\blow_line.c
# End Source File
# Begin Source File

SOURCE=.\effect2\bubble_cm.c
# End Source File
# Begin Source File

SOURCE=.\effect2\bubble_line.c
# End Source File
# Begin Source File

SOURCE=.\effect2\bubble_many.c
# End Source File
# Begin Source File

SOURCE=.\effect2\bubble_motion.c
# End Source File
# Begin Source File

SOURCE=.\effect2\check_water_level.c
# End Source File
# Begin Source File

SOURCE=.\effect2\circle_splush.c
# End Source File
# Begin Source File

SOURCE=.\effect2\crush_parts.c
# End Source File
# Begin Source File

SOURCE=.\effect2\delay_tmplight.c
# End Source File
# Begin Source File

SOURCE=.\effect2\drop_body_splush.c
# End Source File
# Begin Source File

SOURCE=.\effect2\drop_body_splush_prog.c
# End Source File
# Begin Source File

SOURCE=.\effect2\dust_area.c
# End Source File
# Begin Source File

SOURCE=.\effect2\dust_cm.c
# End Source File
# Begin Source File

SOURCE=.\effect2\dynamic_flow.c
# End Source File
# Begin Source File

SOURCE=.\effect2\floor_light_man.c
# End Source File
# Begin Source File

SOURCE=.\effect2\gas_line.c
# End Source File
# Begin Source File

SOURCE=.\effect2\inter_line_blade.c
# End Source File
# Begin Source File

SOURCE=.\effect2\line_brust.c
# End Source File
# Begin Source File

SOURCE=.\effect2\liner_gun_init.c
# End Source File
# Begin Source File

SOURCE=.\effect2\liner_gun_plasma.c
# End Source File
# Begin Source File

SOURCE=.\effect2\liner_gun_plasma_flush.c
# End Source File
# Begin Source File

SOURCE=.\effect2\liner_plasma_small.c
# End Source File
# Begin Source File

SOURCE=.\effect2\metal_blow_water.c
# End Source File
# Begin Source File

SOURCE=.\effect2\point_tidal.c
# End Source File
# Begin Source File

SOURCE=.\effect2\prism_bullet.c
# End Source File
# Begin Source File

SOURCE=.\effect2\rain_cm_con.c
# End Source File
# Begin Source File

SOURCE=.\effect2\ray_sphere.c
# End Source File
# Begin Source File

SOURCE=.\effect2\ripple_bubble.c
# End Source File
# Begin Source File

SOURCE=.\effect2\ripple_man.c
# End Source File
# Begin Source File

SOURCE=.\effect2\ripple_stable.c
# End Source File
# Begin Source File

SOURCE=.\effect2\short_spark.c
# End Source File
# Begin Source File

SOURCE=.\effect2\side_flare.c
# End Source File
# Begin Source File

SOURCE=.\effect2\slit_light.c
# End Source File
# Begin Source File

SOURCE=.\effect2\slit_light2.c
# End Source File
# Begin Source File

SOURCE=.\effect2\sonic_wave.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_big.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_man.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_plane.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_surface_gravity_man.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_surface_man.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_tidal.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_tidal_parts.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_tidal_parts2.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_tidal_parts3.c
# End Source File
# Begin Source File

SOURCE=.\effect2\splush_tidal_parts4.c
# End Source File
# Begin Source File

SOURCE=.\effect2\spot_blur_line.c
# End Source File
# Begin Source File

SOURCE=.\effect2\str_disp.c
# End Source File
# Begin Source File

SOURCE=.\effect2\wall_tidal.c
# End Source File
# Begin Source File

SOURCE=.\effect2\water_drops.c
# End Source File
# Begin Source File

SOURCE=.\effect2\water_front.c
# End Source File
# Begin Source File

SOURCE=.\effect2\water_front_prim.c
# End Source File
# Begin Source File

SOURCE=.\effect2\window_light.c
# End Source File
# End Group
# Begin Group "effect3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\effect3\auto_splush_vertical.c
# End Source File
# Begin Source File

SOURCE=.\effect3\blood_drop_demo.c
# End Source File
# Begin Source File

SOURCE=.\effect3\blood_strip_demo.c
# End Source File
# Begin Source File

SOURCE=.\effect3\bomb_bubble.c
# End Source File
# Begin Source File

SOURCE=.\effect3\bomb_in_water.c
# End Source File
# Begin Source File

SOURCE=.\effect3\bomb_sphere.c
# End Source File
# Begin Source File

SOURCE=.\effect3\breath_demo.c
# End Source File
# Begin Source File

SOURCE=.\effect3\bubble_bullet.c
# End Source File
# Begin Source File

SOURCE=.\effect3\death_dive_splush.c
# End Source File
# Begin Source File

SOURCE=.\effect3\dive_flow.c
# End Source File
# Begin Source File

SOURCE=.\effect3\door_lamp.c
# End Source File
# Begin Source File

SOURCE=.\effect3\drops_many_cm.c
# End Source File
# Begin Source File

SOURCE=.\effect3\dust_cone.c
# End Source File
# Begin Source File

SOURCE=.\effect3\dynamic_scar.c
# End Source File
# Begin Source File

SOURCE=.\effect3\elevator_lamp.c
# End Source File
# Begin Source File

SOURCE=.\effect3\explosion_controler.c
# End Source File
# Begin Source File

SOURCE=.\effect3\explosion_parts.c
# End Source File
# Begin Source File

SOURCE=.\effect3\fluid_flow_man.c
# End Source File
# Begin Source File

SOURCE=.\effect3\fluid_flow_parts.c
# End Source File
# Begin Source File

SOURCE=.\effect3\flying_smoke_slow.c
# End Source File
# Begin Source File

SOURCE=.\effect3\fog_control.c
# End Source File
# Begin Source File

SOURCE=.\effect3\gas2_pers_fast.c
# End Source File
# Begin Source File

SOURCE=.\effect3\gas_in_water.c
# End Source File
# Begin Source File

SOURCE=.\effect3\gas_pers_fast.c
# End Source File
# Begin Source File

SOURCE=.\effect3\hitech_face_blood.c
# End Source File
# Begin Source File

SOURCE=.\effect3\kirari_cm.c
# End Source File
# Begin Source File

SOURCE=.\effect3\kirari_water_cm.c
# End Source File
# Begin Source File

SOURCE=.\effect3\kirari_water_sun.c
# End Source File
# Begin Source File

SOURCE=.\effect3\kirari_water_sun2.c
# End Source File
# Begin Source File

SOURCE=.\effect3\mountain_splush.c
# End Source File
# Begin Source File

SOURCE=.\effect3\mountain_splush_spread.c
# End Source File
# Begin Source File

SOURCE=.\effect3\node_lamp.c
# End Source File
# Begin Source File

SOURCE=.\effect3\peter_blood.c
# End Source File
# Begin Source File

SOURCE=.\effect3\poly_hola_upper.c
# End Source File
# Begin Source File

SOURCE=.\effect3\pool_water.c
# End Source File
# Begin Source File

SOURCE=.\effect3\pool_water_stable.c
# End Source File
# Begin Source File

SOURCE=.\effect3\raiden_mask_bubble.c
# End Source File
# Begin Source File

SOURCE=.\effect3\ray_missile_launcher.c
# End Source File
# Begin Source File

SOURCE=.\effect3\ray_missile_shower.c
# End Source File
# Begin Source File

SOURCE=.\effect3\ripple_strip.c
# End Source File
# Begin Source File

SOURCE=.\effect3\slow_man.c
# End Source File
# Begin Source File

SOURCE=.\effect3\smoke2_strip.c
# End Source File
# Begin Source File

SOURCE=.\effect3\smoke_in_water.c
# End Source File
# Begin Source File

SOURCE=.\effect3\smoke_mitsukoshi.c
# End Source File
# Begin Source File

SOURCE=.\effect3\smoke_mitsukoshi_man.c
# End Source File
# Begin Source File

SOURCE=.\effect3\smoke_strip.c
# End Source File
# Begin Source File

SOURCE=.\effect3\smoke_strip_control.c
# End Source File
# Begin Source File

SOURCE=.\effect3\splush_rsprt.c
# End Source File
# Begin Source File

SOURCE=.\effect3\splush_rsprt_hari.c
# End Source File
# Begin Source File

SOURCE=.\effect3\splush_vertical.c
# End Source File
# Begin Source File

SOURCE=.\effect3\sprt_hola.c
# End Source File
# Begin Source File

SOURCE=.\effect3\sprt_hola_upper.c
# End Source File
# Begin Source File

SOURCE=.\effect3\strip_dome.c
# End Source File
# Begin Source File

SOURCE=.\effect3\strip_hola.c
# End Source File
# Begin Source File

SOURCE=.\effect3\vamp_drop_splash.c
# End Source File
# Begin Source File

SOURCE=.\effect3\vamp_hola.c
# End Source File
# Begin Source File

SOURCE=.\effect3\vamp_knife_fix.c
# End Source File
# Begin Source File

SOURCE=.\effect3\vapor.c
# End Source File
# Begin Source File

SOURCE=.\effect3\view_3d.c
# End Source File
# Begin Source File

SOURCE=.\effect3\w_mine_lamp.c
# End Source File
# Begin Source File

SOURCE=.\effect3\water_reflect.c
# End Source File
# Begin Source File

SOURCE=.\effect3\water_reflect_wall.c
# End Source File
# Begin Source File

SOURCE=.\effect3\water_surface.c
# End Source File
# Begin Source File

SOURCE=.\effect3\water_surface2.c
# End Source File
# End Group
# Begin Group "etc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\etc\ok_util.c
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\effect3\explosion.h
# End Source File
# End Group
# End Target
# Begin Group "Header Files No. 1"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Project
