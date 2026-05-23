//----------------------------------------------------------------------------
// CDrawableRenderEntity.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Frontend/RenderObject/CRenderObject.h"

//----------------------------------------------------------------------------

class CShader;
class CEvaluatorUpdateData;
class CShaderVertexDataBinding;
class CDrawableMasterRenderEntity;

//----------------------------------------------------------------------------

class RENDERER_API CDrawableRenderEntity : public CRenderObject
{
public:
   virtual ~CDrawableRenderEntity();

   // This is called from the shaders after they have set up the material state and called PreRenderGeometry
   // All this function should do is bind vertex streams according to the specified vertex format
   // and do the DrawPrimitive calls.
   // NOTE: This function can be called multiple times for multi pass rendering purposes
   virtual void                  RenderGeometry(CShaderVertexDataBinding const & vertexDataBinding) = 0;

   virtual CMatrix34 const &     GetTransform() const = 0;
   virtual void                  SetTransform( CMatrix34 const & transform ) = 0;

   // returns material associated with this render entity
   CShader *                     GetShader() const { return mpShaderInstance; }
   // Allow instance to be reset for objects that modify shaders
   void                          SetShader(CShader* pShaderInstance )               { mpShaderInstance = pShaderInstance; }

   CDrawableMasterRenderEntity * const 
                                 Owner()                                            { return mpOwner; }
   CDrawableMasterRenderEntity const * const 
                                 GetOwner() const                                   { return mpOwner; }

protected:
   explicit CDrawableRenderEntity( EType const type );

private:
   CShader*                      mpShaderInstance;

protected:
   CDrawableMasterRenderEntity * mpOwner;
};

