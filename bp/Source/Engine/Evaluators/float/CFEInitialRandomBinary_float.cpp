#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialRandomBinary_float(gfloatEvaluatorFactoriesEngine, CFEInitialRandomBinary_float::kType_0x639EBA9C, CFEInitialRandomBinary_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialRandomBinary_float::CFEInitialRandomBinary_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialRandomBinary_float_ctor:73EBF1C4:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEInitialRandomBinary_float_ctor:73EBF1C4 *** Machine generated code - do not edit *** $End$:CFEInitialRandomBinary_float_ctor:73EBF1C4
}

CFEInitialRandomBinary_float::~CFEInitialRandomBinary_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialRandomBinary_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mInitialValue);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialRandomBinary_float_body:AF02DA73:F60117F6 *** Machine generated code - do not edit ***
uint32 const CFEInitialRandomBinary_float::kType_0x639EBA9C = 0x639EBA9C;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialRandomBinary_float::GetType() const
{
   return kType_0x639EBA9C;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialRandomBinary_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialRandomBinary_float * pEval = new CFEInitialRandomBinary_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialRandomBinary_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_ValueA:   // 0xF5DC1850
         {
            if (!applyProperties.UpdateOnly())
            {
               mValueA.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValueA.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_ValueB:   // 0x6CD549EA
         {
            if (!applyProperties.UpdateOnly())
            {
               mValueB.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValueB.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialRandomBinary_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialRandomBinary_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mValueA)
   {
      uint32 const evalPtr = mValueA->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValueA) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mValueB)
   {
      uint32 const evalPtr = mValueB->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValueB) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialRandomBinary_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mValueA) mValueA->PostLoadUpdate();
   if (mValueB) mValueB->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialRandomBinary_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mValueA.get()) mValueA->Initialize(updateData);
   if (mValueB.get()) mValueB->Initialize(updateData);
//End:CFEInitialRandomBinary_float_body:AF02DA73 *** Machine generated code - do not edit *** $End$:CFEInitialRandomBinary_float_body:AF02DA73

   if (updateData.mpRandom->RangeInt(0, 1) == 0)
   {
      updateData.mDataBlock.Value(mInitialValue) =  mValueA->GetValue(updateData);
   }
   else
   {
      updateData.mDataBlock.Value(mInitialValue) =  mValueB->GetValue(updateData);
   }
}
#endif //NO_EVALUATOR_INITIALIZER


