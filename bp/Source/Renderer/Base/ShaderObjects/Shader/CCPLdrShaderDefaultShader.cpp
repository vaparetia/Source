#include "StdAfx.h"

#include "CCPLdrShaderDefaultShader.h"
#include "../ComponentPropertiesCommonCppIncludes.h"

//Start:CCPLdrShaderDefaultShader_body:E93A0C43:B132BEED *** Machine generated code - do not edit ***
//----------------------------------------------------------------------------

CCPLdrShaderDefaultShader::CCPLdrShaderDefaultShader()
{
   mAlbedoColor = CColorf(1.0f, 1.0f, 1.0f, 1.0f);
   mSpecularColor = CColorf(0.0f, 0.0f, 0.0f, 1.0f);
   mIncandescenceColor = CColorf(0.0f, 0.0f, 0.0f, 1.0f);
   mIncandescence2Color = CColorf(0.0f, 0.0f, 0.0f, 1.0f);
   mEnvironmentMapColor = CColorf(1.0f, 1.0f, 1.0f, 1.0f);
   mModulateEnvironmentMapByDiffuse = false;
   mDepthTest = kDT_Enabled;
}

//----------------------------------------------------------------------------

CCPLdrShaderDefaultShader::~CCPLdrShaderDefaultShader()
{
}

//----------------------------------------------------------------------------

char const * CCPLdrShaderDefaultShader::GetComponentTypeName() const
{
   return "DefaultShader";
}

//----------------------------------------------------------------------------

uint32 CCPLdrShaderDefaultShader::GetComponentType() const
{
   return kComponentPropertiesType;  // 0x283F6879;
}

//----------------------------------------------------------------------------

CGameObjectComponentProperties * CCPLdrShaderDefaultShader::clone() const
{
   return new CCPLdrShaderDefaultShader(*this);
}

//----------------------------------------------------------------------------

void CCPLdrShaderDefaultShader::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   (allocator); (applyProperties);
   if (applyProperties.mStream.ReadBool())
   {
      mEditorId = TComponentEditorId(applyProperties.mStream);
   }
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_AlbedoTexture:   // 0xA5B246C7
         {
            mAlbedoTexture = applyProperties.mStream.ReadString();
            break;
         }
         case kP_AlbedoColor:   // 0xBAEAD149
         {
            mAlbedoColor = CColorf(applyProperties.mStream);
            break;
         }
         case kP_SpecularTexture:   // 0xDF3C1B89
         {
            mSpecularTexture = applyProperties.mStream.ReadString();
            break;
         }
         case kP_SpecularColor:   // 0xB3C351C5
         {
            mSpecularColor = CColorf(applyProperties.mStream);
            break;
         }
         case kP_IncandescenceTexture:   // 0x869C3177
         {
            mIncandescenceTexture = applyProperties.mStream.ReadString();
            break;
         }
         case kP_IncandescenceColor:   // 0xC8B27BDD
         {
            mIncandescenceColor = CColorf(applyProperties.mStream);
            break;
         }
         case kP_IncandescenceColorEvaluator:   // 0xA9D9F354
         {
            if (!applyProperties.UpdateOnly())
            {
               mIncandescenceColorEvaluator.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mIncandescenceColorEvaluator.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Incandescence2Texture:   // 0xFB650C1C
         {
            mIncandescence2Texture = applyProperties.mStream.ReadString();
            break;
         }
         case kP_Incandescence2Color:   // 0xBF738245
         {
            mIncandescence2Color = CColorf(applyProperties.mStream);
            break;
         }
         case kP_NormalMapTexture:   // 0x03AD47DD
         {
            mNormalMapTexture = applyProperties.mStream.ReadString();
            break;
         }
         case kP_EnvironmentMap:   // 0x90FB6B27
         {
            mEnvironmentMap = applyProperties.mStream.ReadString();
            break;
         }
         case kP_EnvironmentMapColor:   // 0xF081F2D0
         {
            mEnvironmentMapColor = CColorf(applyProperties.mStream);
            break;
         }
         case kP_EnvironmentMapMask:   // 0xA17D747A
         {
            mEnvironmentMapMask = applyProperties.mStream.ReadString();
            break;
         }
         case kP_ModulateEnvironmentMapByDiffuse:   // 0x526E20D9
         {
            mModulateEnvironmentMapByDiffuse = applyProperties.mStream.ReadBool();
            break;
         }
         case kP_UvOffset:   // 0x1838A99F
         {
            if (!applyProperties.UpdateOnly())
            {
               mUvOffset.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mUvOffset.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_DepthTest:   // 0x71EC4524
         {
            uint32 const enumFourCC = applyProperties.mStream.ReadUint32();
            switch (enumFourCC)
            {
               case 0x2EB3AB3C:   // kDT_Enabled - Depth test enabled
                  mDepthTest = kDT_Enabled;
                  break;
               case 0x413A1F4D:   // kDT_Disabled - Depth test disabled
                  mDepthTest = kDT_Disabled;
                  break;
            }
            break;
         }
         case kP_CommonLightmapProperties:   // 0x5A505511
         {
            mCommonLightmapProperties.ApplyProperties(applyProperties, allocator);
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in component properties loader CCPLdrShaderDefaultShader.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

void CCPLdrShaderDefaultShader::PostLoadUpdate()
{
   // Add code to assign default components if no components were found in the loaded property list.
   // Also lock resources if present and call PostLoadUpdate on child properties.
   // Check for empty asset string
   if (mAlbedoTexture.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mAlbedoTexture);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mAlbedoTexture_Resource = newResource;
   }
   else
   {
      mAlbedoTexture_Resource.reset();
   }
   // Check for empty asset string
   if (mSpecularTexture.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mSpecularTexture);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mSpecularTexture_Resource = newResource;
   }
   else
   {
      mSpecularTexture_Resource.reset();
   }
   // Check for empty asset string
   if (mIncandescenceTexture.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mIncandescenceTexture);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mIncandescenceTexture_Resource = newResource;
   }
   else
   {
      mIncandescenceTexture_Resource.reset();
   }
   if (mIncandescenceColorEvaluator) mIncandescenceColorEvaluator->PostLoadUpdate();
   // Check for empty asset string
   if (mIncandescence2Texture.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mIncandescence2Texture);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mIncandescence2Texture_Resource = newResource;
   }
   else
   {
      mIncandescence2Texture_Resource.reset();
   }
   // Check for empty asset string
   if (mNormalMapTexture.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mNormalMapTexture);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mNormalMapTexture_Resource = newResource;
   }
   else
   {
      mNormalMapTexture_Resource.reset();
   }
   // Check for empty asset string
   if (mEnvironmentMap.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mEnvironmentMap);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mEnvironmentMap_Resource = newResource;
   }
   else
   {
      mEnvironmentMap_Resource.reset();
   }
   // Check for empty asset string
   if (mEnvironmentMapMask.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mEnvironmentMapMask);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mEnvironmentMapMask_Resource = newResource;
   }
   else
   {
      mEnvironmentMapMask_Resource.reset();
   }
   if (mUvOffset) mUvOffset->PostLoadUpdate();
   mCommonLightmapProperties.PostLoadUpdate();
}

//----------------------------------------------------------------------------

void CCPLdrShaderDefaultShader::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
(updateData);

   if (mIncandescenceColorEvaluator) mIncandescenceColorEvaluator->Initialize(updateData);
   if (mUvOffset) mUvOffset->Initialize(updateData);
   mCommonLightmapProperties.InitializeEvaluators(updateData);
}

//End:CCPLdrShaderDefaultShader_body:E93A0C43 *** Machine generated code - do not edit *** $End$:CCPLdrShaderDefaultShader_body:E93A0C43


