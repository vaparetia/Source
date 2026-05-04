#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCMPVEAttractor_modifierPV(gmodifierPVEvaluatorFactoriesEngine, CMPVEAttractor_modifierPV::kType_0x45440AE0, CMPVEAttractor_modifierPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CMPVEAttractor_modifierPV::CMPVEAttractor_modifierPV(CEvaluatorAllocator &allocator)
: IModifierPVEvaluator()
//Start:CMPVEAttractor_modifierPV_ctor:81C2D66F:34008957 *** Machine generated code - do not edit ***
{
   mDistanceFalloff = kDF_None;
//End:CMPVEAttractor_modifierPV_ctor:81C2D66F *** Machine generated code - do not edit *** $End$:CMPVEAttractor_modifierPV_ctor:81C2D66F
}

CMPVEAttractor_modifierPV::~CMPVEAttractor_modifierPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

uint32 CMPVEAttractor_modifierPV::ApplyModifier(CEvaluatorUpdateData const &updateData, void *pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const
{
   (pTempData);
   CVector3 positionDelta(position);
   if (mCenterOffset) 
   {
      positionDelta -= mCenterOffset->GetValue(updateData);
   }

   real32 const distance = positionDelta.GetLength();
   if (distance < gkEpsilon32)
   {
      return kMF_None;
   }

   real32 strength = -1.0f * mStrength->GetValue(updateData);
   // Calculate falloff
   switch (mDistanceFalloff)
   {
      case kDF_None:
         break;
      case kDF_Linear:
         {
            real32 radius = 0.0f;
            if (mFalloffRadius)
            {
               radius = mFalloffRadius->GetValue(updateData);
            }
            if (radius > 0.0f)
            {
               real32 const attenuation = MathUtils::ClampMinMax(1.0f - (distance / radius), 0.0f, 1.0f);
               strength *= attenuation;
            }
         }
       break;
      case kDF_Square:
         {
            real32 radius = 0.0f;
            if (mFalloffRadius)
            {
               radius = mFalloffRadius->GetValue(updateData);
            }
            if (radius > 0.0f)
            {
               real32 attenuation = MathUtils::ClampMinMax(1.0f - (distance / radius), 0.0f, 1.0f);
               attenuation *= attenuation;
               strength *= attenuation;
            }
         }
       break;
      case kDF_InvLinear:
         {
            real32 radius = 0.0f;
            if (mFalloffRadius)
            {
               radius = mFalloffRadius->GetValue(updateData);
            }
            if (radius > 0.0f)
            {
               real32 const attenuation = MathUtils::ClampMinMax((distance / radius), 0.0f, 1.0f);
               strength *= attenuation;
            }
         }
       break;
      case kDF_InvSquare:
         {
            real32 radius = 0.0f;
            if (mFalloffRadius)
            {
               radius = mFalloffRadius->GetValue(updateData);
            }
            if (radius > 0.0f)
            {
               real32 attenuation = MathUtils::ClampMinMax((distance / radius), 0.0f, 1.0f);
               attenuation *= attenuation;
               strength *= attenuation;
            }
         }
       break;
   }
   // Normalize 
   positionDelta /= distance;

   // Apply attraction
   velocity += positionDelta * strength * deltaTime;

   return kMF_None;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CMPVEAttractor_modifierPV_body:5D2BFDD8:3D63D55B *** Machine generated code - do not edit ***
uint32 const CMPVEAttractor_modifierPV::kType_0x45440AE0 = 0x45440AE0;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CMPVEAttractor_modifierPV::GetType() const
{
   return kType_0x45440AE0;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CMPVEAttractor_modifierPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CMPVEAttractor_modifierPV * pEval = new CMPVEAttractor_modifierPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CMPVEAttractor_modifierPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Strength:   // 0x2FCF838E
         {
            if (!applyProperties.UpdateOnly())
            {
               mStrength.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mStrength.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_CenterOffset:   // 0x73523E97
         {
            if (!applyProperties.UpdateOnly())
            {
               mCenterOffset.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mCenterOffset.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_DistanceFalloff:   // 0x2B8063CE
         {
            uint32 const enumFourCC = applyProperties.mStream.ReadUint32();
            switch (enumFourCC)
            {
               case 0xC543F368:   // kDF_None - None
                  mDistanceFalloff = kDF_None;
                  break;
               case 0xC82C8BE8:   // kDF_Linear - Linear
                  mDistanceFalloff = kDF_Linear;
                  break;
               case 0x8115BF2:   // kDF_Square - Square
                  mDistanceFalloff = kDF_Square;
                  break;
               case 0x20915F7B:   // kDF_InvLinear - InvLinear
                  mDistanceFalloff = kDF_InvLinear;
                  break;
               case 0xE0AC8F61:   // kDF_InvSquare - InvSquare
                  mDistanceFalloff = kDF_InvSquare;
                  break;
            }
            break;
         }
         case kP_FalloffRadius:   // 0xC0D49CCB
         {
            if (!applyProperties.UpdateOnly())
            {
               mFalloffRadius.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mFalloffRadius.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CMPVEAttractor_modifierPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CMPVEAttractor_modifierPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mStrength)
   {
      uint32 const evalPtr = mStrength->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mStrength) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mCenterOffset)
   {
      uint32 const evalPtr = mCenterOffset->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mCenterOffset) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mFalloffRadius)
   {
      uint32 const evalPtr = mFalloffRadius->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mFalloffRadius) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CMPVEAttractor_modifierPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mStrength) mStrength->PostLoadUpdate();
   if (mCenterOffset) mCenterOffset->PostLoadUpdate();
   if (mFalloffRadius) mFalloffRadius->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CMPVEAttractor_modifierPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mStrength.get()) mStrength->Initialize(updateData);
   if (mCenterOffset.get()) mCenterOffset->Initialize(updateData);
   if (mFalloffRadius.get()) mFalloffRadius->Initialize(updateData);
//End:CMPVEAttractor_modifierPV_body:5D2BFDD8 *** Machine generated code - do not edit *** $End$:CMPVEAttractor_modifierPV_body:5D2BFDD8
}

#endif //NO_EVALUATOR_INITIALIZER

