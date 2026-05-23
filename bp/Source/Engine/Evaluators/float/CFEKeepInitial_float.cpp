#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEKeepInitial_float(gfloatEvaluatorFactoriesEngine, CFEKeepInitial_float::kType_0x84659304, CFEKeepInitial_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEKeepInitial_float::CFEKeepInitial_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEKeepInitial_float_ctor:D6C08447:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEKeepInitial_float_ctor:D6C08447 *** Machine generated code - do not edit *** $End$:CFEKeepInitial_float_ctor:D6C08447
}

CFEKeepInitial_float::~CFEKeepInitial_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEKeepInitial_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mInitialValue);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEKeepInitial_float_body:0A29AFF0:6E440EA6 *** Machine generated code - do not edit ***
uint32 const CFEKeepInitial_float::kType_0x84659304 = 0x84659304;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEKeepInitial_float::GetType() const
{
   return kType_0x84659304;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEKeepInitial_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEKeepInitial_float * pEval = new CFEKeepInitial_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEKeepInitial_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Value:   // 0xDCB67730
         {
            if (!applyProperties.UpdateOnly())
            {
               mValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEKeepInitial_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEKeepInitial_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mValue)
   {
      uint32 const evalPtr = mValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEKeepInitial_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mValue) mValue->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEKeepInitial_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mValue.get()) mValue->Initialize(updateData);
//End:CFEKeepInitial_float_body:0A29AFF0 *** Machine generated code - do not edit *** $End$:CFEKeepInitial_float_body:0A29AFF0
   updateData.mDataBlock.SetValue(mInitialValue, mValue->GetValue(updateData));
}

#endif //NO_EVALUATOR_INITIALIZER

