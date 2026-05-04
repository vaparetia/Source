#define STRICT

#include <stdlib.h>
#include <string.h>


#include <SFDLINUX.h>

int	ConvertMotionFile( char *out_path, char *in_file, char *mdl_file )
{
	char       szPath[_MAX_PATH];
	char       szDrive[_MAX_DRIVE];
	char       szDir[_MAX_DIR];
	char       szFName[_MAX_FNAME];
	P3DXYZ     xyzScale;
	HP3DACTOR  hP3DActor;
	HP3DMOTION hP3DMotion;
	HP3DMODEL  hP3DModel;

#if 1
	if((hP3DModel = P3DReadFileModel( mdl_file, NULL, 0)) == NULL)
		return 0;

	if(((hP3DActor = P3DCreateActor()) == NULL)
		|| ((P3DSetActorModel(hP3DActor, hP3DModel)) == 0))
	{
		P3DDestroyModel(hP3DModel);
		return 0;
	}

	if((hP3DMotion = P3DReadFileMotion(
		hP3DActor, FALSE, in_file, NULL, 0)) == NULL)
	{
		P3DDestroyActor(hP3DActor);
		return 0;
	}
#else
	if((hP3DActor = P3DReadFileActor( in_file, NULL, 0)) == NULL)
		return 0;
#endif

	xyzScale.x = 10.0F;
	xyzScale.y = 10.0F;
	xyzScale.z = 10.0F;

	if((hP3DMotion = P3DGetFirstMotion(hP3DActor)) != NULL)
	{
		do
		{
			_makepath(szPath,
				NULL, out_path, P3DMotion(hP3DMotion)->szName, "mtn");

			if(P3DWriteFileMETALGEAR2Motion(szPath, &xyzScale,
				hP3DActor, P3DMotion(hP3DMotion)->szName, NULL, 0) == 0)
			{
				P3DDestroyActor(hP3DActor);
				return 0;
			}
		}
		while((hP3DMotion = P3DGetNextMotion(hP3DMotion)) != NULL);
	}

	P3DDestroyActor(hP3DActor);

	return 255;
}


int	ConvertMotionFile2( char *out_file, char *in_file, char *mdl_file )
{
	char       szPath[_MAX_PATH];
	char       szDrive[_MAX_DRIVE];
	char       szDir[_MAX_DIR];
	char       szFName[_MAX_FNAME];
	P3DXYZ     xyzScale;
	HP3DACTOR  hP3DActor;
	HP3DMOTION hP3DMotion;
	HP3DMODEL  hP3DModel;

	if((hP3DModel = P3DReadFileModel( mdl_file, NULL, 0)) == NULL)
		return 0;

	if(((hP3DActor = P3DCreateActor()) == NULL)
		|| ((P3DSetActorModel(hP3DActor, hP3DModel)) == 0))
	{
		P3DDestroyModel(hP3DModel);
		return 0;
	}

	if((hP3DMotion = P3DReadFileMotion(
		hP3DActor, FALSE, in_file, NULL, 0)) == NULL)
	{
		P3DDestroyActor(hP3DActor);
		return 0;
	}

	xyzScale.x = 10.0F;
	xyzScale.y = 10.0F;
	xyzScale.z = 10.0F;

	if((hP3DMotion = P3DGetFirstMotion(hP3DActor)) != NULL)
	{
		printf("%s\n", P3DMotion(hP3DMotion)->szName);
		/* １ファイル１モーションを前提とするので */
		if(P3DWriteFileMETALGEAR2Motion2( out_file, &xyzScale,
									   hP3DActor, P3DMotion(hP3DMotion)->szName, NULL, 0) == 0){
			P3DDestroyActor(hP3DActor);
			return 0;
		}
	}

	P3DDestroyActor(hP3DActor);

	return 255;
}

