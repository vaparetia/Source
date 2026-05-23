#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFERandomBinary_float(gfloatEvaluatorFactoriesEngine, CFERandomBinary_float::kType_0x5698FEE1, CFERandomBinary_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFERandomBinary_float::CFERandomBinary_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFERandomBinary_float_ctor:C3402A40:15D54739 *** Machine generated code - do not edit ***
{
//End:CFERandomBinary_float_ctor:C3402A40 *** Machine generated code - do not edit *** $End$:CFERandomBinary_float_ctor:C3402A40
}

CFERandomBinary_float::~CFERandomBinary_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFERandomBinary_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   if (updateData.mpRandom->RangeInt(0, 1) == 0)
   {
      return mValueA->GetValue(updateData);
   }
   return mValueB->GetValue(updateData);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFERandomBinary_float_body:1FA901F7:C871EEC6 *** Machine generated code - do not edit ***
uint32 const CFERandomBinary_float::kType_0x5698FEE1 = 0x5698FEE1;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFERandomBinary_float::GetType() const
{
   return kType_0x5698FEE1;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFERandomBinary_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFERandomBinary_float * pEval = new CFERandomBinary_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFERandomBinary_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFERandomBinary_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFERandomBinary_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CFERandomBinary_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mValueA) mValueA->PostLoadUpdate();
   if (mValueB) mValueB->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFERandomBinary_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mValueA.get()) mValueA->Initialize(updateData);
   if (mValueB.get()) mValueB->Initialize(updateData);
//End:CFERandomBinary_float_body:1FA901F7 *** Machine generated code - do not edit *** $End$:CFERandomBinary_float_body:1FA901F7
}
#endif //NO_EVALUATOR_INITIALIZER


