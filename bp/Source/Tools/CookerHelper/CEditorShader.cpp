//------------------------------------------------------------------------------------------
// CEditorShader.cpp
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CEditorShader.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CEditorShader::CEditorShader( boost::shared_ptr<char> const & data,
                              int const dataSize, 
                              std::vector<CGuid> const & lightIds,
                              std::vector<std::string> const & lightmaps )
:  mData( data )
,  mDataSize( dataSize )
,  mLightIds( lightIds )
,  mLightMaps( lightmaps )
{
}

