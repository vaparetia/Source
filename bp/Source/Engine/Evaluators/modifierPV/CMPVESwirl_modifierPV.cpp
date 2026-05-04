#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCMPVESwirl_modifierPV(gmodifierPVEvaluatorFactoriesEngine, CMPVESwirl_modifierPV::kType_0x6AB7DE9B, CMPVESwirl_modifierPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

//----------------------------------------------------------------------------

CMPVESwirl_modifierPV::CMPVESwirl_modifierPV(CEvaluatorAllocator &allocator)
: IModifierPVEvaluator()
//Start:CMPVESwirl_modifierPV_ctor:454D7C5D:EAB9EC97 *** Machine generated code - do not edit ***
{
   mDistanceFalloff = kDF_None;
   mAttenuateStrength = true;
   mAttenuateDrag = false;
//End:CMPVESwirl_modifierPV_ctor:454D7C5D *** Machine generated code - do not edit *** $End$:CMPVESwirl_modifierPV_ctor:454D7C5D
}

//----------------------------------------------------------------------------

CMPVESwirl_modifierPV::~CMPVESwirl_modifierPV()
{
}

//----------------------------------------------------------------------------

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

uint32 CMPVESwirl_modifierPV::ApplyModifier(CEvaluatorUpdateData const &updateData, void *pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const
{
   (pTempData);
   // Need to apply pow function to deltaTime, doesn't play nice with variable frame rates
   real32 lerpAmount = mDrag->GetValue(updateData) * deltaTime;

   CVector3 upAxis(mUpAxis->GetValue(updateData));

   real32 strength = mStrength->GetValue(updateData);
   // Calculate falloff
   switch (mDistanceFalloff)
   {
      case kDF_None:
         break;
      case kDF_Linear:
         {
            real32 const distance = position.GetLength();
            real32 radius = 0.0f;
            if (mFalloffRadius)
            {
               radius = mFalloffRadius->GetValue(updateData);
            }
            if (radius > 0.0f)
            {
               real32 const attenuation = MathUtils::ClampMinMax(1.0f - (distance / radius), 0.0f, 1.0f);
               if (mAttenuateDrag) lerpAmount *= attenuation;
               if (mAttenuateStrength) strength *= attenuation;
            }
         }
       break;
      case kDF_Square:
         {
            real32 const distance = position.GetLength();
            real32 radius = 0.0f;
            if (mFalloffRadius)
            {
               radius = mFalloffRadius->GetValue(updateData);
            }
            if (radius > 0.0f)
            {
               real32 attenuation = MathUtils::ClampMinMax(1.0f - (distance / radius), 0.0f, 1.0f);
               attenuation *= attenuation;
               if (mAttenuateDrag) lerpAmount *= attenuation;
               if (mAttenuateStrength) strength *= attenuation;
            }
         }
       break;
   }


   CVector3 const centerPos(CVector3::Zero());
   CVector3 const positionDelta(centerPos - position);
   CVector3 const perpDist(positionDelta - (upAxis * CVector3::Dot(positionDelta, upAxis)));
   CVector3 const appliedVel(CVector3::Cross(upAxis, perpDist) * strength);

   velocity = CVector3::Lerp(velocity, upAxis * CVector3::Dot(upAxis, velocity) + appliedVel, lerpAmount);

   return kMF_None;
}

#endif //NO_EVALUATOR_GET_VALUE

//----------------------------------------------------------------------------

//Start:CMPVESwirl_modifierPV_body:99A457EA:20785433 *** Machine generated code - do not edit ***
uint32 const CMPVESwirl_modifierPV::kType_0x6AB7DE9B = 0x6AB7DE9B;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CMPVESwirl_modifierPV::GetType() const
{
   return kType_0x6AB7DE9B;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CMPVESwirl_modifierPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CMPVESwirl_modifierPV * pEval = new CMPVESwirl_modifierPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CMPVESwirl_modifierPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_UpAxis:   // 0x102A2EF0
         {
            if (!applyProperties.UpdateOnly())
            {
               mUpAxis.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mUpAxis.get(), applyProperties, size, allocator);
            }
            break;
         }
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
         case kP_Drag:   // 0xCD770255
         {
            if (!applyProperties.UpdateOnly())
            {
               mDrag.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mDrag.get(), applyProperties, size, allocator);
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
            }
            break;
         }
         case kP_AttenuateStrength:   // 0xE61B2A03
         {
            mAttenuateStrength = applyProperties.mStream.ReadBool();
            break;
         }
         case kP_AttenuateDrag:   // 0x4194B252
         {
            mAttenuateDrag = applyProperties.mStream.ReadBool();
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CMPVESwirl_modifierPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CMPVESwirl_modifierPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mUpAxis)
   {
      uint32 const evalPtr = mUpAxis->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mUpAxis) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mStrength)
   {
      uint32 const evalPtr = mStrength->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mStrength) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mDrag)
   {
      uint32 const evalPtr = mDrag->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mDrag) - ((uint8*) this));
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

void CMPVESwirl_modifierPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mUpAxis) mUpAxis->PostLoadUpdate();
   if (mStrength) mStrength->PostLoadUpdate();
   if (mDrag) mDrag->PostLoadUpdate();
   if (mFalloffRadius) mFalloffRadius->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CMPVESwirl_modifierPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mUpAxis.get()) mUpAxis->Initialize(updateData);
   if (mStrength.get()) mStrength->Initialize(updateData);
   if (mDrag.get()) mDrag->Initialize(updateData);
   if (mFalloffRadius.get()) mFalloffRadius->Initialize(updateData);
//End:CMPVESwirl_modifierPV_body:99A457EA *** Machine generated code - do not edit *** $End$:CMPVESwirl_modifierPV_body:99A457EA
}

#endif //NO_EVALUATOR_INITIALIZER

