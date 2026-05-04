#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEGaussianRandomS_float(gfloatEvaluatorFactoriesEngine, CFEGaussianRandomS_float::kType_0xB4855225, CFEGaussianRandomS_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEGaussianRandomS_float::CFEGaussianRandomS_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFEGaussianRandomS_float_ctor:E7BA3924:9AA70D4E *** Machine generated code - do not edit ***
{
   mMean = 0.0f;
   mVariance = 1.0f;
//End:CFEGaussianRandomS_float_ctor:E7BA3924 *** Machine generated code - do not edit *** $End$:CFEGaussianRandomS_float_ctor:E7BA3924
}

CFEGaussianRandomS_float::~CFEGaussianRandomS_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEGaussianRandomS_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mpRandom->RandomFloatRangeGD(mMean, mVariance);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEGaussianRandomS_float_body:3B531293:81C36993 *** Machine generated code - do not edit ***
uint32 const CFEGaussianRandomS_float::kType_0xB4855225 = 0xB4855225;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEGaussianRandomS_float::GetType() const
{
   return kType_0xB4855225;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEGaussianRandomS_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEGaussianRandomS_float * pEval = new CFEGaussianRandomS_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEGaussianRandomS_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Mean:   // 0xD0CA408E
         {
            mMean = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Variance:   // 0xBBB90070
         {
            mVariance = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEGaussianRandomS_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEGaussianRandomS_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEGaussianRandomS_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEGaussianRandomS_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFEGaussianRandomS_float_body:3B531293 *** Machine generated code - do not edit *** $End$:CFEGaussianRandomS_float_body:3B531293
}

#endif //NO_EVALUATOR_INITIALIZER

