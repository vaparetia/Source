#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialGaussianRandomS_float(gfloatEvaluatorFactoriesEngine, CFEInitialGaussianRandomS_float::kType_0x95DA8627, CFEInitialGaussianRandomS_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialGaussianRandomS_float::CFEInitialGaussianRandomS_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialGaussianRandomS_float_ctor:224EEC7C:9AA70D4E *** Machine generated code - do not edit ***
{
   mMean = 0.0f;
   mVariance = 1.0f;
//End:CFEInitialGaussianRandomS_float_ctor:224EEC7C *** Machine generated code - do not edit *** $End$:CFEInitialGaussianRandomS_float_ctor:224EEC7C
}

CFEInitialGaussianRandomS_float::~CFEInitialGaussianRandomS_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialGaussianRandomS_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mInitialValue);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialGaussianRandomS_float_body:FEA7C7CB:87B67CB3 *** Machine generated code - do not edit ***
uint32 const CFEInitialGaussianRandomS_float::kType_0x95DA8627 = 0x95DA8627;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialGaussianRandomS_float::GetType() const
{
   return kType_0x95DA8627;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialGaussianRandomS_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialGaussianRandomS_float * pEval = new CFEInitialGaussianRandomS_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialGaussianRandomS_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialGaussianRandomS_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialGaussianRandomS_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialGaussianRandomS_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialGaussianRandomS_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFEInitialGaussianRandomS_float_body:FEA7C7CB *** Machine generated code - do not edit *** $End$:CFEInitialGaussianRandomS_float_body:FEA7C7CB
   real32 const initialValue = updateData.mpRandom->RandomFloatRangeGD(mMean, mVariance);
   updateData.mDataBlock.SetValue(mInitialValue, initialValue);
}

#endif //NO_EVALUATOR_INITIALIZER

