#pragma once

#include "ICGLdrShader.h"
#include "boost/optional.hpp"
#include "Engine/Resource/IResourcePool.h"
#include "Engine/Graphics/CColorf.h"

#include "Renderer/Base/ShaderObjects/CShaderObjectFactory.h"
#include "../Properties/CPOLdrShaderCommonLightmapProperties.h"

class CBaseTexture;
class CInputStream;
class IVectorEvaluator;
class IColorEvaluator;

//Start:CCPLdrShaderDefaultShader_declaration:544B9F4D:05C59ECD *** Machine generated code - do not edit ***
class RENDERER_API CCPLdrShaderDefaultShader : public ICGLdrShader
{
public:
   CCPLdrShaderDefaultShader();
   virtual ~CCPLdrShaderDefaultShader();

#pragma region "Component Property Internal Glue Code"
   virtual void                                    ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void                                    PostLoadUpdate();
   virtual uint32                                  GetComponentType() const;
   virtual char const *                            GetComponentTypeName() const;
   virtual CGameObjectComponentProperties *        clone() const;

   void                                            InitializeEvaluators(CEvaluatorUpdateData &updateData) const;

   static const uint32                             kComponentPropertiesType = 0x283F6879;

   enum EProperties
   {
      kP_AlbedoTexture                                 = 0xA5B246C7,
      kP_AlbedoColor                                   = 0xBAEAD149,
      kP_SpecularTexture                               = 0xDF3C1B89,
      kP_SpecularColor                                 = 0xB3C351C5,
      kP_IncandescenceTexture                          = 0x869C3177,
      kP_IncandescenceColor                            = 0xC8B27BDD,
      kP_IncandescenceColorEvaluator                   = 0xA9D9F354,
      kP_Incandescence2Texture                         = 0xFB650C1C,
      kP_Incandescence2Color                           = 0xBF738245,
      kP_NormalMapTexture                              = 0x03AD47DD,
      kP_EnvironmentMap                                = 0x90FB6B27,
      kP_EnvironmentMapColor                           = 0xF081F2D0,
      kP_EnvironmentMapMask                            = 0xA17D747A,
      kP_ModulateEnvironmentMapByDiffuse               = 0x526E20D9,
      kP_UvOffset                                      = 0x1838A99F,
      kP_DepthTest                                     = 0x71EC4524,
      kP_CommonLightmapProperties                      = 0x5A505511,

      kP_Terminator                                    = 0xFFFFFFFF
   };
#pragma endregion

   enum EDepthTest
   {
      kDT_Enabled,
      kDT_Disabled,

      EDepthTest_Count
   };

   // Regular component properties
   std::string                                     mAlbedoTexture;
   boost::optional<TResource<CBaseTexture> >       mAlbedoTexture_Resource;
   CColorf                                         mAlbedoColor;
   std::string                                     mSpecularTexture;
   boost::optional<TResource<CBaseTexture> >       mSpecularTexture_Resource;
   CColorf                                         mSpecularColor;
   std::string                                     mIncandescenceTexture;
   boost::optional<TResource<CBaseTexture> >       mIncandescenceTexture_Resource;
   CColorf                                         mIncandescenceColor;
   boost::shared_ptr<IColorEvaluator>              mIncandescenceColorEvaluator;
   std::string                                     mIncandescence2Texture;
   boost::optional<TResource<CBaseTexture> >       mIncandescence2Texture_Resource;
   CColorf                                         mIncandescence2Color;
   std::string                                     mNormalMapTexture;
   boost::optional<TResource<CBaseTexture> >       mNormalMapTexture_Resource;
   std::string                                     mEnvironmentMap;
   boost::optional<TResource<CBaseTexture> >       mEnvironmentMap_Resource;
   CColorf                                         mEnvironmentMapColor;
   std::string                                     mEnvironmentMapMask;
   boost::optional<TResource<CBaseTexture> >       mEnvironmentMapMask_Resource;
   bool                                            mModulateEnvironmentMapByDiffuse;
   boost::shared_ptr<IVectorEvaluator>             mUvOffset;
   EDepthTest                                      mDepthTest;
   CPOLdrShaderCommonLightmapProperties            mCommonLightmapProperties;
};

//End:CCPLdrShaderDefaultShader_declaration:544B9F4D *** Machine generated code - do not edit *** $End$:CCPLdrShaderDefaultShader_declaration:544B9F4D

