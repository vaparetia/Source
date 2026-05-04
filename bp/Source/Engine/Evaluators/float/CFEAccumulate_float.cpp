#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEAccumulate_float(gfloatEvaluatorFactoriesEngine, CFEAccumulate_float::kType_0xD2096B0D, CFEAccumulate_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEAccumulate_float::CFEAccumulate_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mAccumulatedValue(allocator.AllocFloat())
, mLastTime(allocator.AllocFloat())
//Start:CFEAccumulate_float_ctor:07CC9BC4:85884FC3 *** Machine generated code - do not edit ***
{
   mScale = 1.0f;
   mWrap = 0.0f;
   mOffset = 0.0f;
//End:CFEAccumulate_float_ctor:07CC9BC4 *** Machine generated code - do not edit *** $End$:CFEAccumulate_float_ctor:07CC9BC4
}

CFEAccumulate_float::~CFEAccumulate_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEAccumulate_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CEvaluatorDataBlock & dataBlock = const_cast<CEvaluatorDataBlock&>(updateData.mDataBlock);

   real32 & currentValue = dataBlock.Value(mAccumulatedValue);

   real32 & lastTime = dataBlock.Value(mLastTime);

   real32 const deltaTime = updateData.mLocalTime - lastTime;
   if ( deltaTime > 0.0f )
   {
      lastTime = updateData.mLocalTime;

      real32 const input = mInput->GetValue(updateData);
      real32 const timeScaledInput = input * deltaTime;
      currentValue += timeScaledInput;
   }

   real32 output = currentValue * mScale;
   if (mWrap > 0)
   {
      output = fmodf(output, mWrap);
   }
   output += mOffset;

   return output;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEAccumulate_float_body:DB25B073:7673D74B *** Machine generated code - do not edit ***
uint32 const CFEAccumulate_float::kType_0xD2096B0D = 0xD2096B0D;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEAccumulate_float::GetType() const
{
   return kType_0xD2096B0D;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEAccumulate_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEAccumulate_float * pEval = new CFEAccumulate_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEAccumulate_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_Scale:   // 0x2D870A80
         {
            mScale = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Wrap:   // 0x0C087FE3
         {
            mWrap = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Offset:   // 0x5EA6CFE6
         {
            mOffset = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEAccumulate_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEAccumulate_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEAccumulate_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEAccumulate_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFEAccumulate_float_body:DB25B073 *** Machine generated code - do not edit *** $End$:CFEAccumulate_float_body:DB25B073
   updateData.mDataBlock.SetValue(mAccumulatedValue, 0.0f);
   updateData.mDataBlock.SetValue(mLastTime, updateData.mLocalTime);
}
#endif //NO_EVALUATOR_INITIALIZER


