//----------------------------------------------------------------------------
// CRenderObject.h
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//----------------------------------------------------------------------------

const int kMaxRenderObjectSize = 512;

//----------------------------------------------------------------------------

class RENDERER_API CRenderObject
{
public:
   enum EType
   {
      kType_Light,

      kType_RenderBounds,
      kType_RenderEntity,

      kType_Invalid = -1
   };

public:
   virtual ~CRenderObject();

   EType const GetType() const { return mType; }
   
   void SetSibling(CRenderObject * pSibling) { mpSibling = pSibling; }
   CRenderObject * Sibling() { return mpSibling; }

protected:
   explicit CRenderObject(EType const type);

private:
   // type of this node
   EType mType;

   // points to next sibling node
   CRenderObject*             mpSibling;
};

