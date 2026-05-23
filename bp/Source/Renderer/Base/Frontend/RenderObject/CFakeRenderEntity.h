//----------------------------------------------------------------------------
// CFakeRenderEntity.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Evaluators/CEvaluatorUpdateData.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"

//----------------------------------------------------------------------------

class CFakeRenderEntity : public CDrawableRenderEntity
{
public:
   CFakeRenderEntity()
      :  CDrawableRenderEntity(kType_RenderEntity)
      ,  mTransform(CMatrix34::Identity())
   {
   }

   virtual void                  RenderGeometry(CShaderVertexDataBinding const & /*vertexDataBinding*/)  {}

   // returns material associated with this render entity
   virtual CShader *             GetShaderInstance() const                             { return NULL; }
   virtual CMatrix34 const &     GetTransform() const                                  { return mTransform; }
   virtual void                  SetTransform(CMatrix34 const & /*transform*/)         {}

public:
   CMatrix34         mTransform;
};