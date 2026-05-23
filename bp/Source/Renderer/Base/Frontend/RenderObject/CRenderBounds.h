//----------------------------------------------------------------------------
// CRenderBounds.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Primitives/CAABox.h"
#include "Renderer/Base/Frontend/RenderObject/CRenderObject.h"

//----------------------------------------------------------------------------

class CDrawableMasterRenderEntity;

//----------------------------------------------------------------------------

class RENDERER_API CRenderBounds : public CRenderObject
{
public:
   CRenderBounds()
   :  CRenderObject(kType_RenderBounds)
   ,  mpMasterEntity(NULL)
   ,  mBounds(CAABox::MakeMaxInvertedBox())
   ,  mpChild(NULL)
   {
   }

   CRenderBounds(CDrawableMasterRenderEntity* pMasterEntity)
      :  CRenderObject(kType_RenderBounds)
      ,  mpMasterEntity(pMasterEntity)
      ,  mBounds(CAABox::MakeMaxInvertedBox())
      ,  mpChild(NULL)
   {
   }

   virtual ~CRenderBounds();

   void SetMasterEntity(CDrawableMasterRenderEntity * pEntity) { mpMasterEntity = pEntity; }
   CDrawableMasterRenderEntity * MasterEntity() { return mpMasterEntity; }

   CAABox const & GetBounds() const { return mBounds; }
   void SetBounds(CAABox const & bounds) { mBounds = bounds; }

   void AddChild(CRenderObject* pChild);

   CRenderObject * Child() { return mpChild; }

private:
   // points to entity that provides the transform for this bounds node
   CDrawableMasterRenderEntity*  mpMasterEntity;
   // local space bounds
   CAABox                        mBounds;
   // points to first child
   CRenderObject*                mpChild;
};

