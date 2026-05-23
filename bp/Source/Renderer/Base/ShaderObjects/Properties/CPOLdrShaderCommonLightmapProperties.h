#pragma once

#include "Engine/Resource/IResourcePool.h"
#include "boost/optional.hpp"

class CBaseTexture;

//Start:CPOLdrShaderCommonLightmapProperties_declaration:2BC0E98E:422EFE0E *** Machine generated code - do not edit ***
class CEvaluatorAllocator;
class CEvaluatorUpdateData;
class IEvaluatorFactory;
class IEvaluator;
class IBoolEvaluator;
class IIntEvaluator;
class IFloatEvaluator;
class IColorEvaluator;
class IVectorEvaluator;
class CGameObjectApplyPropertiesData;
class ICustomEvaluator;
class IUserDataEvaluator;

class CPOLdrShaderCommonLightmapProperties
{
public:
   CPOLdrShaderCommonLightmapProperties();
   ~CPOLdrShaderCommonLightmapProperties();

   enum EProperties
   {
      kP_LightmapTexture_Flat                          = 0x2EAB4F40,
      kP_LightmapTexture_X                             = 0x3699EA33,
      kP_LightmapTexture_Y                             = 0x419EDAA5,
      kP_LightmapTexture_Z                             = 0xD8978B1F,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   void	ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   void	PostLoadUpdate();
   void	InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

   std::string                                     mLightmapTexture_Flat;
   boost::optional<TResource<CBaseTexture> >       mLightmapTexture_Flat_Resource;
   std::string                                     mLightmapTexture_X;
   boost::optional<TResource<CBaseTexture> >       mLightmapTexture_X_Resource;
   std::string                                     mLightmapTexture_Y;
   boost::optional<TResource<CBaseTexture> >       mLightmapTexture_Y_Resource;
   std::string                                     mLightmapTexture_Z;
   boost::optional<TResource<CBaseTexture> >       mLightmapTexture_Z_Resource;
};

//End:CPOLdrShaderCommonLightmapProperties_declaration:2BC0E98E *** Machine generated code - do not edit *** $End$:CPOLdrShaderCommonLightmapProperties_declaration:2BC0E98E
