#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEClampMinMax_float(gfloatEvaluatorFactoriesEngine, CFEClampMinMax_float::kType_0x77796F8C, CFEClampMinMax_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEClampMinMax_float::CFEClampMinMax_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFEClampMinMax_float_ctor:B14B3E61:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEClampMinMax_float_ctor:B14B3E61 *** Machine generated code - do not edit *** $End$:CFEClampMinMax_float_ctor:B14B3E61
}

CFEClampMinMax_float::~CFEClampMinMax_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEClampMinMax_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const min = mMin->GetValue(updateData);
   real32 const max = mMax->GetValue(updateData);
   real32 value = mInput->GetValue(updateData);
   value = bpe::max_val(min, bpe::min_val(max, value));
   return value;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEClampMinMax_float_body:6DA215D6:525D9D7E *** Machine generated code - do not edit ***
uint32 const CFEClampMinMax_float::kType_0x77796F8C = 0x77796F8C;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEClampMinMax_float::GetType() const
{
   return kType_0x77796F8C;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEClampMinMax_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEClampMinMax_float * pEval = new CFEClampMinMax_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEClampMinMax_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEClampMinMax_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEClampMinMax_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CFEClampMinMax_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
   if (mMin) mMin->PostLoadUpdate();
   if (mMax) mMax->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEClampMinMax_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
   if (mMin.get()) mMin->Initialize(updateData);
   if (mMax.get()) mMax->Initialize(updateData);
//End:CFEClampMinMax_float_body:6DA215D6 *** Machine generated code - do not edit *** $End$:CFEClampMinMax_float_body:6DA215D6
}
#endif //NO_EVALUATOR_INITIALIZER


