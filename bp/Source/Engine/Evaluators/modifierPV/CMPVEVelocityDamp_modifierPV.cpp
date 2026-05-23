#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCMPVEVelocityDamp_modifierPV(gmodifierPVEvaluatorFactoriesEngine, CMPVEVelocityDamp_modifierPV::kType_0x44FB1D59, CMPVEVelocityDamp_modifierPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CMPVEVelocityDamp_modifierPV::CMPVEVelocityDamp_modifierPV(CEvaluatorAllocator &allocator)
: IModifierPVEvaluator()
//Start:CMPVEVelocityDamp_modifierPV_ctor:C4B5BA16:80C6CEF8 *** Machine generated code - do not edit ***
{
   mClampMinVelocity = false;
   mClampMaxVelocity = false;
//End:CMPVEVelocityDamp_modifierPV_ctor:C4B5BA16 *** Machine generated code - do not edit *** $End$:CMPVEVelocityDamp_modifierPV_ctor:C4B5BA16
}

CMPVEVelocityDamp_modifierPV::~CMPVEVelocityDamp_modifierPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

uint32 CMPVEVelocityDamp_modifierPV::ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const
{
   real32 const speedSq = velocity.GetLengthSquared();
   real32 const originalSpeed = sqrtf(speedSq);

   if (originalSpeed < gkEpsilon32)
   {
      // Moving too slow to normalize / modify velocity
      return kMF_None;
   }

   real32 const linearVelocityDamp = mLinearDampFactor->GetValue(updateData);
   real32 const squaredVelocityDamp = mSquaredDampFactor->GetValue(updateData);
   real32 newSpeed = originalSpeed;
   newSpeed -= linearVelocityDamp * originalSpeed * deltaTime;
   // Clamp max damping to 99% of current velocity otherwise we'll get -ve speeds
   // This is a little arbitary but serves as a fail safe
   newSpeed -= bpe::min_val(squaredVelocityDamp * speedSq * deltaTime, newSpeed * 0.99f);
   if (mClampMinVelocity)
   {
      real32 const minSpeed = mMinVelocity->GetValue(updateData);
      newSpeed = bpe::max_val(newSpeed, minSpeed);
   }

   if (mClampMaxVelocity)
   {
      real32 const maxSpeed = mMaxVelocity->GetValue(updateData);
      newSpeed = bpe::min_val(newSpeed, maxSpeed);
   }
   // Normalize and scale
   velocity *= newSpeed / originalSpeed;

   return kMF_None;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CMPVEVelocityDamp_modifierPV_body:185C91A1:2616200B *** Machine generated code - do not edit ***
uint32 const CMPVEVelocityDamp_modifierPV::kType_0x44FB1D59 = 0x44FB1D59;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CMPVEVelocityDamp_modifierPV::GetType() const
{
   return kType_0x44FB1D59;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CMPVEVelocityDamp_modifierPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CMPVEVelocityDamp_modifierPV * pEval = new CMPVEVelocityDamp_modifierPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CMPVEVelocityDamp_modifierPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_LinearDampFactor:   // 0x30115BB3
         {
            if (!applyProperties.UpdateOnly())
            {
               mLinearDampFactor.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mLinearDampFactor.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_SquaredDampFactor:   // 0x2FA856C4
         {
            if (!applyProperties.UpdateOnly())
            {
               mSquaredDampFactor.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mSquaredDampFactor.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_ClampMinVelocity:   // 0x1682FA23
         {
            mClampMinVelocity = applyProperties.mStream.ReadBool();
            break;
         }
         case kP_ClampMaxVelocity:   // 0xDA8F812C
         {
            mClampMaxVelocity = applyProperties.mStream.ReadBool();
            break;
         }
         case kP_MinVelocity:   // 0xED84FB1E
         {
            if (!applyProperties.UpdateOnly())
            {
               mMinVelocity.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mMinVelocity.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_MaxVelocity:   // 0x21898011
         {
            if (!applyProperties.UpdateOnly())
            {
               mMaxVelocity.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mMaxVelocity.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CMPVEVelocityDamp_modifierPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CMPVEVelocityDamp_modifierPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mLinearDampFactor)
   {
      uint32 const evalPtr = mLinearDampFactor->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mLinearDampFactor) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mSquaredDampFactor)
   {
      uint32 const evalPtr = mSquaredDampFactor->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mSquaredDampFactor) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mMinVelocity)
   {
      uint32 const evalPtr = mMinVelocity->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mMinVelocity) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mMaxVelocity)
   {
      uint32 const evalPtr = mMaxVelocity->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mMaxVelocity) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CMPVEVelocityDamp_modifierPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mLinearDampFactor) mLinearDampFactor->PostLoadUpdate();
   if (mSquaredDampFactor) mSquaredDampFactor->PostLoadUpdate();
   if (mMinVelocity) mMinVelocity->PostLoadUpdate();
   if (mMaxVelocity) mMaxVelocity->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CMPVEVelocityDamp_modifierPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mLinearDampFactor.get()) mLinearDampFactor->Initialize(updateData);
   if (mSquaredDampFactor.get()) mSquaredDampFactor->Initialize(updateData);
   if (mMinVelocity.get()) mMinVelocity->Initialize(updateData);
   if (mMaxVelocity.get()) mMaxVelocity->Initialize(updateData);
//End:CMPVEVelocityDamp_modifierPV_body:185C91A1 *** Machine generated code - do not edit *** $End$:CMPVEVelocityDamp_modifierPV_body:185C91A1
}
#endif //NO_EVALUATOR_INITIALIZER


