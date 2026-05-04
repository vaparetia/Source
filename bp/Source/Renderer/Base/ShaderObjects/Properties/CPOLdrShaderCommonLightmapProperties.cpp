//Start:CPOLdrShaderCommonLightmapProperties_body:A29AF868:853AFE21 *** Machine generated code - do not edit ***
//----------------------------------------------------------------------------

CPOLdrShaderCommonLightmapProperties::CPOLdrShaderCommonLightmapProperties()
{
}

//----------------------------------------------------------------------------

CPOLdrShaderCommonLightmapProperties::~CPOLdrShaderCommonLightmapProperties()
{
}

//----------------------------------------------------------------------------

void CPOLdrShaderCommonLightmapProperties::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   (allocator); (applyProperties);
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_LightmapTexture_Flat:   // 0x2EAB4F40
         {
            mLightmapTexture_Flat = applyProperties.mStream.ReadString();
            break;
         }
         case kP_LightmapTexture_X:   // 0x3699EA33
         {
            mLightmapTexture_X = applyProperties.mStream.ReadString();
            break;
         }
         case kP_LightmapTexture_Y:   // 0x419EDAA5
         {
            mLightmapTexture_Y = applyProperties.mStream.ReadString();
            break;
         }
         case kP_LightmapTexture_Z:   // 0xD8978B1F
         {
            mLightmapTexture_Z = applyProperties.mStream.ReadString();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in property object loader CPOLdrShaderCommonLightmapProperties.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

void CPOLdrShaderCommonLightmapProperties::PostLoadUpdate()
{
   // Check for empty asset string
   if (mLightmapTexture_Flat.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mLightmapTexture_Flat);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mLightmapTexture_Flat_Resource = newResource;
   }
   else
   {
      mLightmapTexture_Flat_Resource.reset();
   }
   // Check for empty asset string
   if (mLightmapTexture_X.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mLightmapTexture_X);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mLightmapTexture_X_Resource = newResource;
   }
   else
   {
      mLightmapTexture_X_Resource.reset();
   }
   // Check for empty asset string
   if (mLightmapTexture_Y.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mLightmapTexture_Y);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mLightmapTexture_Y_Resource = newResource;
   }
   else
   {
      mLightmapTexture_Y_Resource.reset();
   }
   // Check for empty asset string
   if (mLightmapTexture_Z.size())
   {
      // Lock dependencies, build resource.
      CResId const resId(mLightmapTexture_Z);

      CResource newResource(gpResources->GetResource(resId));
      newResource.Lock();

      mLightmapTexture_Z_Resource = newResource;
   }
   else
   {
      mLightmapTexture_Z_Resource.reset();
   }
}

//----------------------------------------------------------------------------

void CPOLdrShaderCommonLightmapProperties::InitializeEvaluators(CEvaluatorUpdateData &updateData) const
{
   (updateData);

}

//End:CPOLdrShaderCommonLightmapProperties_body:A29AF868 *** Machine generated code - do not edit *** $End$:CPOLdrShaderCommonLightmapProperties_body:A29AF868
