#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialGaussianRandom_float(gfloatEvaluatorFactoriesEngine, CFEInitialGaussianRandom_float::kType_0x2FF0584E, CFEInitialGaussianRandom_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialGaussianRandom_float::CFEInitialGaussianRandom_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialGaussianRandom_float_ctor:1A5CA065:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEInitialGaussianRandom_float_ctor:1A5CA065 *** Machine generated code - do not edit *** $End$:CFEInitialGaussianRandom_float_ctor:1A5CA065
}

CFEInitialGaussianRandom_float::~CFEInitialGaussianRandom_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialGaussianRandom_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return updateData.mDataBlock.GetValue(mInitialValue);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialGaussianRandom_float_body:C6B58BD2:F97EC1BC *** Machine generated code - do not edit ***
uint32 const CFEInitialGaussianRandom_float::kType_0x2FF0584E = 0x2FF0584E;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialGaussianRandom_float::GetType() const
{
   return kType_0x2FF0584E;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialGaussianRandom_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialGaussianRandom_float * pEval = new CFEInitialGaussianRandom_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialGaussianRandom_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            if (!applyProperties.UpdateOnly())
            {
               mMean.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mMean.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Variance:   // 0xBBB90070
         {
            if (!applyProperties.UpdateOnly())
            {
               mVariance.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mVariance.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialGaussianRandom_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialGaussianRandom_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mMean)
   {
      uint32 const evalPtr = mMean->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mMean) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mVariance)
   {
      uint32 const evalPtr = mVariance->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mVariance) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialGaussianRandom_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mMean) mMean->PostLoadUpdate();
   if (mVariance) mVariance->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialGaussianRandom_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mMean.get()) mMean->Initialize(updateData);
   if (mVariance.get()) mVariance->Initialize(updateData);
//End:CFEInitialGaussianRandom_float_body:C6B58BD2 *** Machine generated code - do not edit *** $End$:CFEInitialGaussianRandom_float_body:C6B58BD2
   real32 const mean = mMean->GetValue(updateData);
   real32 const variance = mVariance->GetValue(updateData);
   updateData.mDataBlock.SetValue(mInitialValue, updateData.mpRandom->RandomFloatRangeGD(mean, variance));
}

#endif //NO_EVALUATOR_INITIALIZER

