#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFESpline_float(gfloatEvaluatorFactoriesEngine, CFESpline_float::kType_0x1BB90D00, CFESpline_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFESpline_float::CFESpline_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFESpline_float_ctor:197FF796:15D54739 *** Machine generated code - do not edit ***
{
//End:CFESpline_float_ctor:197FF796 *** Machine generated code - do not edit *** $End$:CFESpline_float_ctor:197FF796

}

CFESpline_float::~CFESpline_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFESpline_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   SSplineEvaluationCache cache;
   return mSplineData.Evaluate(mInput->GetValue(updateData), &cache);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFESpline_float_body:C596DC21:7844B928 *** Machine generated code - do not edit ***
uint32 const CFESpline_float::kType_0x1BB90D00 = 0x1BB90D00;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFESpline_float::GetType() const
{
   return kType_0x1BB90D00;
}

//----------------------------------------------------------------------------

void CFESpline_float::GetInfo(STypeInfo &info) const
{
   IFloatEvaluator::GetInfo(info);
   
   // Get min/max time ranges for all spines   
   info.mTimeRangeStart = gkReal32Max;
   info.mTimeRangeEnd = gkReal32Min;
   
   {
      CSpline const &spline = mSplineData;      
      if (spline.mKeys.size() >= 1)
      {
         info.mTimeRangeStart = bpe::min_val(spline.mKeys[0].mTime, info.mTimeRangeStart);
         info.mTimeRangeEnd = bpe::max_val(spline.mKeys[spline.mKeys.size() - 1].mTime, info.mTimeRangeEnd);
      }   
      else
      {
         info.mTimeRangeStart = 0.0f;
         info.mTimeRangeEnd = gkEpsilon32;      // Return small delta to prevent callers having to add logic for zero time ranges.

      }
   }   
   
   if (info.mTimeRangeStart == info.mTimeRangeEnd)
   {   
      // Only 1 key, return small delta to prevent callers having to add logic for zero time ranges.
      info.mTimeRangeEnd = info.mTimeRangeStart * (1.0f + (10.0f * gkEpsilon32));      
   }
}

#endif //NO_EVALUATOR_TYPE

//----------------------------------------------------------------------------

#if !defined(NO_EVALUATOR_LOADER)

IEvaluator * CFESpline_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFESpline_float * pEval = new CFESpline_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFESpline_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_SplineData:   // 0x97C93F7C
         {
            mSplineData.ConstructFromStream(applyProperties.mStream);
            break;
         }
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
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFESpline_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFESpline_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   {
      int const memberPtrOffset = (int) (((uint8*) &mSplineData) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mSplineData.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFESpline_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFESpline_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFESpline_float_body:C596DC21 *** Machine generated code - do not edit *** $End$:CFESpline_float_body:C596DC21

}

#endif //NO_EVALUATOR_INITIALIZER

