//------------------------------------------------------------------------------------------
// CDirList.h
// Bluepoint
// Copyright 2003
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Mechanics/FastDelegate/FastDelegate.h"

//------------------------------------------------------------------------------------------

class CDirList
{
public:
   typedef fastdelegate::FastDelegate1<std::string const &, bool> TRecurseDelegate;

   ENGINE_API static void GetFiles( std::string const & path,
                                    std::string const & extension,
                                    bool const recursive,
                                    std::vector< std::string > & output );

   ENGINE_API static void GetFiles( std::string const & path,
                                    std::string const & extension,
                                    TRecurseDelegate const &recurseDirectory,
                                    std::vector< std::string > & output);

};

//------------------------------------------------------------------------------------------

