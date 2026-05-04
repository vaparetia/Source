//------------------------------------------------------------------------------------------
// CEditorShader.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/System/CGuid.h"
#include "boost/shared_ptr.hpp"

//------------------------------------------------------------------------------------------

class CEditorShader
{
public:
   CEditorShader( boost::shared_ptr<char> const & data,
                  int const dataSize,
                  std::vector<CGuid> const & lightIds,
                  std::vector<std::string> const & lightMaps );

public:
   boost::shared_ptr<char>       mData;
   int                           mDataSize;
   bpe::vector_s<CGuid>          mLightIds;
   bpe::vector_s<std::string>    mLightMaps;
};
