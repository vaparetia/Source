//------------------------------------------------------------------------------------------
// XmlFactory.cpp
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Mechanics/TinyXml/XmlFactory.h"
#include "Engine/Mechanics/TinyXml/TinyXml.h"
#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Resource/CResourceFactory.h"


//----------------------------------------------------------------------------

void FXmlFactory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource)
{   
   BPE_VERIFY( buildData.mpMemory != NULL, false, "must have memory" );
   CMemoryInputStream stream( buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App );

   std::string xmlText(stream.ReadString());

   TiXmlDocument *pXmlDoc = new TiXmlDocument();
   pXmlDoc->Parse(xmlText.c_str());
            
   returnResource.mpResource = pXmlDoc;
}

//----------------------------------------------------------------------------



