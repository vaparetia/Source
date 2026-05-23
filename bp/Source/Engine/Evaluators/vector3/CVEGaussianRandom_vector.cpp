#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVEGaussianRandom_vector(gvector3EvaluatorFactoriesEngine, CVEGaussianRandom_vector::kType_0x2BD6C203, CVEGaussianRandom_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVEGaussianRandom_vector::CVEGaussianRandom_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVEGaussianRandom_vector_ctor:9707D5C8:15D54739 *** Machine generated code - do not edit ***
{
//End:CVEGaussianRandom_vector_ctor:9707D5C8 *** Machine generated code - do not edit *** $End$:CVEGaussianRandom_vector_ctor:9707D5C8
}

CVEGaussianRandom_vector::~CVEGaussianRandom_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVEGaussianRandom_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
	const CVector3 mean(mMean->GetValue(updateData));
	const CVector3 variance(mVariance->GetValue(updateData));
	return CVector3(
		updateData.mpRandom->RandomFloatRangeGD(mean.GetX(), variance.GetX()),
		updateData.mpRandom->RandomFloatRangeGD(mean.GetY(), variance.GetY()),
		updateData.mpRandom->RandomFloatRangeGD(mean.GetZ(), variance.GetZ())
		);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVEGaussianRandom_vector_body:4BEEFE7F:04C88B2F *** Machine generated code - do not edit ***
uint32 const CVEGaussianRandom_vector::kType_0x2BD6C203 = 0x2BD6C203;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVEGaussianRandom_vector::GetType() const
{
   return kType_0x2BD6C203;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVEGaussianRandom_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVEGaussianRandom_vector * pEval = new CVEGaussianRandom_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVEGaussianRandom_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
               mMean.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
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
               mVariance.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mVariance.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVEGaussianRandom_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVEGaussianRandom_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CVEGaussianRandom_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mMean) mMean->PostLoadUpdate();
   if (mVariance) mVariance->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVEGaussianRandom_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mMean.get()) mMean->Initialize(updateData);
   if (mVariance.get()) mVariance->Initialize(updateData);
//End:CVEGaussianRandom_vector_body:4BEEFE7F *** Machine generated code - do not edit *** $End$:CVEGaussianRandom_vector_body:4BEEFE7F
}
#endif //NO_EVALUATOR_INITIALIZER


