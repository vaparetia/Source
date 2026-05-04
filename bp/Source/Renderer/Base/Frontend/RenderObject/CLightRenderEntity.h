//----------------------------------------------------------------------------
// CLightRenderEntity.h
// Bluepoint
// Copyright 2004
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Renderer/Base/Frontend/RenderObject/CRenderObject.h"

#include "Engine/Graphics/CLight.h"

//----------------------------------------------------------------------------

class CLightRenderEntity : public CRenderObject
{
public:
   RENDERER_API explicit CLightRenderEntity();
   RENDERER_API explicit CLightRenderEntity( CLight const & light );
   RENDERER_API virtual ~CLightRenderEntity();

   void              SetTransform( CMatrix34 const & transform )  { mLight.SetTransform( transform ); }
   CMatrix34 const & GetTransform() const                         { return mLight.GetTransform(); }
   
   CLight const &    GetLight() const { return mLight; }
   CLight &          Light() { return mLight; }

private:
   CLight   mLight;
};


