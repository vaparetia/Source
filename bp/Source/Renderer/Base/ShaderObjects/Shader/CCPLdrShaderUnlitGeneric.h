#pragma once

#include "ICGLdrShader.h"
#include "boost/optional.hpp"
#include "Engine/Resource/CResourceManager.h"

class CBaseTexture;
class CInputStream;
class CEvaluatorUpdateData;

//Start:CCPLdrShaderUnlitGeneric_declaration *** Stub ***
class RENDERER_API CCPLdrShaderUnlitGeneric : public ICGLdrShader
{
public:
   CCPLdrShaderUnlitGeneric();
   virtual ~CCPLdrShaderUnlitGeneric();

   virtual void          ApplyProperties(CGameObjectApplyPropertiesData &, CEvaluatorAllocator &) {}
   virtual void          PostLoadUpdate() {}
   virtual uint32        GetComponentType() const { return kComponentPropertiesType; }
   virtual char const *  GetComponentTypeName() const { return "CCPLdrShaderUnlitGeneric"; }
   virtual CGameObjectComponentProperties * clone() const { return NULL; }

   void InitializeEvaluators(CEvaluatorUpdateData & /*updateData*/) const {}

   static const uint32 kComponentPropertiesType = 0x7A3C1E2F;

   enum EDrawPriority
   {
      kDP_0 = 0,
      kDP_1 = 1,
   };

   EDrawPriority   mDrawPriority;
   bool            mDepthCompare;
   bool            mDepthWrite;
   int             mBlendMode;

   std::string                                  mTexture;
   boost::optional<TResource<CBaseTexture> >    mTexture_Resource;
};
//End:CCPLdrShaderUnlitGeneric_declaration
