#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEFollowValue_float(gfloatEvaluatorFactoriesEngine, CFEFollowValue_float::kType_0x879CB96A, CFEFollowValue_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEFollowValue_float::CFEFollowValue_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mCurrentValue(allocator.AllocFloat())
, mLastTime(allocator.AllocFloat())
//Start:CFEFollowValue_float_ctor:AE7519AD:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEFollowValue_float_ctor:AE7519AD *** Machine generated code - do not edit *** $End$:CFEFollowValue_float_ctor:AE7519AD
}

CFEFollowValue_float::~CFEFollowValue_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEFollowValue_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const input = mInput->GetValue(updateData);
   real32 const speed = bpe::max_val(0.0f, mSpeed->GetValue(updateData));

   CEvaluatorDataBlock & dataBlock = const_cast<CEvaluatorDataBlock&>(updateData.mDataBlock);

   real32 & currentValue = dataBlock.Value(mCurrentValue);

   real32 & lastTime = dataBlock.Value(mLastTime);

   real32 const deltaTime = updateData.mLocalTime - lastTime;
   if( deltaTime > 0.0f )
   {
      lastTime = updateData.mLocalTime;

      real32 const scaledSpeed = speed * deltaTime;

      // Check which "direction" we're approaching the value
      real32 delta = input - currentValue;
      if( delta > 0.0f )
      {
         // Approach input without overshooting
         currentValue = bpe::min_val(currentValue + scaledSpeed, input);
      }
      else
      {
         // Approach input without overshooting
         currentValue = bpe::max_val(currentValue - scaledSpeed, input);
      }
   }

   return currentValue;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEFollowValue_float_body:729C321A:AE43CABA *** Machine generated code - do not edit ***
uint32 const CFEFollowValue_float::kType_0x879CB96A = 0x879CB96A;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEFollowValue_float::GetType() const
{
   return kType_0x879CB96A;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEFollowValue_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEFollowValue_float * pEval = new CFEFollowValue_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEFollowValue_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
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
         case kP_Speed:   // 0xCEE7D1F2
         {
            if (!applyProperties.UpdateOnly())
            {
               mSpeed.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mSpeed.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEFollowValue_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEFollowValue_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mSpeed)
   {
      uint32 const evalPtr = mSpeed->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mSpeed) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEFollowValue_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
   if (mSpeed) mSpeed->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEFollowValue_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
   if (mSpeed.get()) mSpeed->Initialize(updateData);
//End:CFEFollowValue_float_body:729C321A *** Machine generated code - do not edit *** $End$:CFEFollowValue_float_body:729C321A

   updateData.mDataBlock.SetValue(mCurrentValue, mInput->GetValue(updateData));
   updateData.mDataBlock.SetValue(mLastTime, updateData.mLocalTime);
}
#endif //NO_EVALUATOR_INITIALIZER


