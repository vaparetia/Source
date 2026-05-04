#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialRandomS_float(gfloatEvaluatorFactoriesEngine, CFEInitialRandomS_float::kType_0xA41AC064, CFEInitialRandomS_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialRandomS_float::CFEInitialRandomS_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialRandomS_float_ctor:8AF6A867:0A806AD1 *** Machine generated code - do not edit ***
{
   mMin = 0.0f;
   mMax = 1.0f;
//End:CFEInitialRandomS_float_ctor:8AF6A867 *** Machine generated code - do not edit *** $End$:CFEInitialRandomS_float_ctor:8AF6A867
}

CFEInitialRandomS_float::~CFEInitialRandomS_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialRandomS_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mInitialValue);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialRandomS_float_body:561F83D0:42381696 *** Machine generated code - do not edit ***
uint32 const CFEInitialRandomS_float::kType_0xA41AC064 = 0xA41AC064;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialRandomS_float::GetType() const
{
   return kType_0xA41AC064;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialRandomS_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialRandomS_float * pEval = new CFEInitialRandomS_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialRandomS_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            mMin = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Max:   // 0xA53B6469
         {
            mMax = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialRandomS_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialRandomS_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialRandomS_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialRandomS_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFEInitialRandomS_float_body:561F83D0 *** Machine generated code - do not edit *** $End$:CFEInitialRandomS_float_body:561F83D0
   real32 const initialValue = updateData.mpRandom->RangeFloat(mMin, mMax);
   updateData.mDataBlock.SetValue(mInitialValue, initialValue);
}

#endif //NO_EVALUATOR_INITIALIZER

