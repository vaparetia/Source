//----------------------------------------------------------------------------
// CDrawableMasterRenderEntity.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"

#include "Engine/Evaluators/CEvaluatorUpdateData.h"
#include "Engine/Math/CRandom.h"

//----------------------------------------------------------------------------

class RENDERER_API CDrawableMasterRenderEntity : public CDrawableRenderEntity
{
public:
   // PreRenderGeometry is called once per frame prior to the first  RenderGeometry call.
   // It allows you to calculate data that will not change in between multiple draw calls.
   // Any data that is calculated inside this function should not be reliant on the material 
   // used to render the geometry.
   // You can add a task to the task queue to be multithread if it is an expensive call.
   // However, inexpensive calls shouldn't do this because of the overhead.
   // If prerender geometry returns false, it signifies that we have to call this 
   // function again as we're waiting for async prerender tasks to complete.
   virtual bool PreRenderGeometry();

   // this is a time stamp used to identify if this object has prerendered this frame or not
   uint32 const GetPreRenderGeometryTimeStamp() const       { return mPreRenderTimeStamp; }

protected:
   explicit CDrawableMasterRenderEntity(EType const type);
   uint32                     mPreRenderTimeStamp;
};