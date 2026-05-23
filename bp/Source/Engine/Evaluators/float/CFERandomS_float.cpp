#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFERandomS_float(gfloatEvaluatorFactoriesEngine, CFERandomS_float::kType_0x27ADE674, CFERandomS_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFERandomS_float::CFERandomS_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFERandomS_float_ctor:C5FEED26:0A806AD1 *** Machine generated code - do not edit ***
{
   mMin = 0.0f;
   mMax = 1.0f;
//End:CFERandomS_float_ctor:C5FEED26 *** Machine generated code - do not edit *** $End$:CFERandomS_float_ctor:C5FEED26
}

CFERandomS_float::~CFERandomS_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFERandomS_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const value = updateData.mpRandom->RangeFloat(mMin, mMax);
   return value;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFERandomS_float_body:1917C691:C69B6157 *** Machine generated code - do not edit ***
uint32 const CFERandomS_float::kType_0x27ADE674 = 0x27ADE674;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFERandomS_float::GetType() const
{
   return kType_0x27ADE674;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFERandomS_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFERandomS_float * pEval = new CFERandomS_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFERandomS_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFERandomS_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFERandomS_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFERandomS_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFERandomS_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFERandomS_float_body:1917C691 *** Machine generated code - do not edit *** $End$:CFERandomS_float_body:1917C691
}

#endif //NO_EVALUATOR_INITIALIZER

