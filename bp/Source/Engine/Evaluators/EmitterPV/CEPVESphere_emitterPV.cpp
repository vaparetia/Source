#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gemitterPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCEPVESphere_emitterPV(gemitterPVEvaluatorFactoriesEngine, CEPVESphere_emitterPV::kType_0xBA288E1C, CEPVESphere_emitterPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CEPVESphere_emitterPV::CEPVESphere_emitterPV(CEvaluatorAllocator &allocator)
: IEmitterPVEvaluator()
//Start:CEPVESphere_emitterPV_ctor:D6FAB618:15D54739 *** Machine generated code - do not edit ***
{
//End:CEPVESphere_emitterPV_ctor:D6FAB618 *** Machine generated code - do not edit *** $End$:CEPVESphere_emitterPV_ctor:D6FAB618
}

CEPVESphere_emitterPV::~CEPVESphere_emitterPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

void CEPVESphere_emitterPV::GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const
{
   position = updateData.mpRandom->RandomVector3(1.0f);
   velocity = position * mVelocity->GetValue(updateData);
   position *= mRadius->GetValue(updateData);

   if (mCenterOffset) position += mCenterOffset->GetValue(updateData);

   return;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CEPVESphere_emitterPV_body:0A139DAF:46205EC2 *** Machine generated code - do not edit ***
uint32 const CEPVESphere_emitterPV::kType_0xBA288E1C = 0xBA288E1C;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CEPVESphere_emitterPV::GetType() const
{
   return kType_0xBA288E1C;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CEPVESphere_emitterPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CEPVESphere_emitterPV * pEval = new CEPVESphere_emitterPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CEPVESphere_emitterPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Radius:   // 0x3CD06B6C
         {
            if (!applyProperties.UpdateOnly())
            {
               mRadius.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mRadius.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Velocity:   // 0x47AEB4FF
         {
            if (!applyProperties.UpdateOnly())
            {
               mVelocity.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mVelocity.get(), applyProperties, size, allocator);
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
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CEPVESphere_emitterPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CEPVESphere_emitterPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mRadius)
   {
      uint32 const evalPtr = mRadius->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mRadius) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mVelocity)
   {
      uint32 const evalPtr = mVelocity->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mVelocity) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mCenterOffset)
   {
      uint32 const evalPtr = mCenterOffset->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mCenterOffset) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CEPVESphere_emitterPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mRadius) mRadius->PostLoadUpdate();
   if (mVelocity) mVelocity->PostLoadUpdate();
   if (mCenterOffset) mCenterOffset->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CEPVESphere_emitterPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mRadius.get()) mRadius->Initialize(updateData);
   if (mVelocity.get()) mVelocity->Initialize(updateData);
   if (mCenterOffset.get()) mCenterOffset->Initialize(updateData);
//End:CEPVESphere_emitterPV_body:0A139DAF *** Machine generated code - do not edit *** $End$:CEPVESphere_emitterPV_body:0A139DAF
}

#endif //NO_EVALUATOR_INITIALIZER

