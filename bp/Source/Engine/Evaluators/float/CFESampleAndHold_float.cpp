#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFESampleAndHold_float(gfloatEvaluatorFactoriesEngine, CFESampleAndHold_float::kType_0xDB257918, CFESampleAndHold_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFESampleAndHold_float::CFESampleAndHold_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mLastSample(allocator.AllocFloat())
, mLastSampleTime(allocator.AllocFloat())
//Start:CFESampleAndHold_float_ctor:F61A1E4F:15D54739 *** Machine generated code - do not edit ***
{
//End:CFESampleAndHold_float_ctor:F61A1E4F *** Machine generated code - do not edit *** $End$:CFESampleAndHold_float_ctor:F61A1E4F
}

CFESampleAndHold_float::~CFESampleAndHold_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFESampleAndHold_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 timeInput = mInput->GetValue(updateData);
   real32 holdTime = mHold->GetValue(updateData);
   
   if( (updateData.mDataBlock.GetValue(mLastSampleTime) + holdTime) < timeInput )
   {
      real32 lastSample = mSample->GetValue(updateData);
      // This is effectively a cache, so we'll const cast to treat as mutable.
      // Bit of a grey area.
      CEvaluatorDataBlock &dataBlock = const_cast<CEvaluatorDataBlock &>(updateData.mDataBlock);
      dataBlock.SetValue(mLastSample, lastSample);
      dataBlock.SetValue(mLastSampleTime, timeInput);
      return lastSample;
   }

   return updateData.mDataBlock.GetValue(mLastSample);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFESampleAndHold_float_body:2AF335F8:FBC3D1DE *** Machine generated code - do not edit ***
uint32 const CFESampleAndHold_float::kType_0xDB257918 = 0xDB257918;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFESampleAndHold_float::GetType() const
{
   return kType_0xDB257918;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFESampleAndHold_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFESampleAndHold_float * pEval = new CFESampleAndHold_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFESampleAndHold_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Sample:   // 0xF6A773F5
         {
            if (!applyProperties.UpdateOnly())
            {
               mSample.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mSample.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Hold:   // 0xBFF8A239
         {
            if (!applyProperties.UpdateOnly())
            {
               mHold.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mHold.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Input:   // 0x19E91DD3
         {
            if (!applyProperties.UpdateOnly())
            {
               mInput.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInput.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFESampleAndHold_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFESampleAndHold_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mSample)
   {
      uint32 const evalPtr = mSample->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mSample) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mHold)
   {
      uint32 const evalPtr = mHold->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mHold) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFESampleAndHold_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mSample) mSample->PostLoadUpdate();
   if (mHold) mHold->PostLoadUpdate();
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFESampleAndHold_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mSample.get()) mSample->Initialize(updateData);
   if (mHold.get()) mHold->Initialize(updateData);
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFESampleAndHold_float_body:2AF335F8 *** Machine generated code - do not edit *** $End$:CFESampleAndHold_float_body:2AF335F8
   real32 const timeInput = mInput->GetValue(updateData);
   real32 const lastSample = mSample->GetValue(updateData);
   updateData.mDataBlock.SetValue(mLastSample, lastSample);
   updateData.mDataBlock.SetValue(mLastSampleTime, timeInput);
}

#endif //NO_EVALUATOR_INITIALIZER

