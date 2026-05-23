#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialRandom_float(gfloatEvaluatorFactoriesEngine, CFEInitialRandom_float::kType_0x8138F4E0, CFEInitialRandom_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialRandom_float::CFEInitialRandom_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialRandom_float_ctor:C5C7EEC8:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEInitialRandom_float_ctor:C5C7EEC8 *** Machine generated code - do not edit *** $End$:CFEInitialRandom_float_ctor:C5C7EEC8

}

CFEInitialRandom_float::~CFEInitialRandom_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialRandom_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mInitialValue);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialRandom_float_body:192EC57F:829431F7 *** Machine generated code - do not edit ***
uint32 const CFEInitialRandom_float::kType_0x8138F4E0 = 0x8138F4E0;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialRandom_float::GetType() const
{
   return kType_0x8138F4E0;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialRandom_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialRandom_float * pEval = new CFEInitialRandom_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialRandom_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Min:   // 0x99365B30
         {
            if (!applyProperties.UpdateOnly())
            {
               mMin.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mMin.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Max:   // 0xA53B6469
         {
            if (!applyProperties.UpdateOnly())
            {
               mMax.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mMax.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialRandom_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialRandom_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mMin)
   {
      uint32 const evalPtr = mMin->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mMin) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mMax)
   {
      uint32 const evalPtr = mMax->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mMax) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialRandom_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mMin) mMin->PostLoadUpdate();
   if (mMax) mMax->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialRandom_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mMin.get()) mMin->Initialize(updateData);
   if (mMax.get()) mMax->Initialize(updateData);
//End:CFEInitialRandom_float_body:192EC57F *** Machine generated code - do not edit *** $End$:CFEInitialRandom_float_body:192EC57F

   updateData.mDataBlock.Value(mInitialValue) = updateData.mpRandom->RangeFloat(mMin->GetValue(updateData), mMax->GetValue(updateData));
}

#endif //NO_EVALUATOR_INITIALIZER

