#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFERandom_float(gfloatEvaluatorFactoriesEngine, CFERandom_float::kType_0xE59B23F7, CFERandom_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFERandom_float::CFERandom_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFERandom_float_ctor:1270FD6B:15D54739 *** Machine generated code - do not edit ***
{
//End:CFERandom_float_ctor:1270FD6B *** Machine generated code - do not edit *** $End$:CFERandom_float_ctor:1270FD6B
}

CFERandom_float::~CFERandom_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFERandom_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const randomValue = updateData.mpRandom->RangeFloat(mMin->GetValue(updateData), mMax->GetValue(updateData));
   return randomValue;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFERandom_float_body:CE99D6DC:FBEDE845 *** Machine generated code - do not edit ***
uint32 const CFERandom_float::kType_0xE59B23F7 = 0xE59B23F7;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFERandom_float::GetType() const
{
   return kType_0xE59B23F7;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFERandom_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFERandom_float * pEval = new CFERandom_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFERandom_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFERandom_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFERandom_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CFERandom_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mMin) mMin->PostLoadUpdate();
   if (mMax) mMax->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFERandom_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mMin.get()) mMin->Initialize(updateData);
   if (mMax.get()) mMax->Initialize(updateData);
//End:CFERandom_float_body:CE99D6DC *** Machine generated code - do not edit *** $End$:CFERandom_float_body:CE99D6DC
}

#endif //NO_EVALUATOR_INITIALIZER

