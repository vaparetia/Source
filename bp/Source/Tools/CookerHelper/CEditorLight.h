//------------------------------------------------------------------------------------------
// CEditorLight.h
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Graphics/CLight.h"

//------------------------------------------------------------------------------------------

class CXMLTree;

//------------------------------------------------------------------------------------------

class CEditorLight : public CLight
{
public:
   explicit CEditorLight( CXMLTree const & tree );
   void PutTo( CXMLTree & tree ) const;
   
protected:
   explicit CEditorLight( CMatrix34 const & transform,
                          EType const type,
                          CVector3 const & color,
                          real32 const intensity,
                          EFalloff const falloff,
                          CAngle const & coneAngle,
                          CAngle const & penumbraAngle,
                          real32 const dropOff,
                          bool const castShadows );
protected:   
   void ParseTree( CXMLTree const & tree );

};
