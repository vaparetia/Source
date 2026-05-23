#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFESin_float(gfloatEvaluatorFactoriesEngine, CFESin_float::kType_0x4BA4F69F, CFESin_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFESin_float::CFESin_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFESin_float_ctor:F5D82634:96AD6928 *** Machine generated code - do not edit ***
{
   mOutputMagnitude = 1.0f;
   mSinMagnitude = 1.0f;
   mBias = 1.0f;
   mPhase = 0.0f;
   mInputScale = 1.0f;
//End:CFESin_float_ctor:F5D82634 *** Machine generated code - do not edit *** $End$:CFESin_float_ctor:F5D82634
}

CFESin_float::~CFESin_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFESin_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const input = mInput->GetValue(updateData);
   real32 const output = mOutputMagnitude * (mSinMagnitude * (sinf((input * mInputScale * gkDegrees2Radians32) + mPhase)) + mBias);
   return output;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFESin_float_body:29310D83:CF177468 *** Machine generated code - do not edit ***
uint32 const CFESin_float::kType_0x4BA4F69F = 0x4BA4F69F;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFESin_float::GetType() const
{
   return kType_0x4BA4F69F;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFESin_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFESin_float * pEval = new CFESin_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFESin_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_OutputMagnitude:   // 0x58C3C196
         {
            mOutputMagnitude = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_SinMagnitude:   // 0x85F839E1
         {
            mSinMagnitude = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Bias:   // 0xE2B5C565
         {
            mBias = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Phase:   // 0x707CF9CF
         {
            mPhase = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_InputScale:   // 0x7C92C27C
         {
            mInputScale = applyProperties.mStream.ReadReal32();
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFESin_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFESin_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFESin_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFESin_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFESin_float_body:29310D83 *** Machine generated code - do not edit *** $End$:CFESin_float_body:29310D83
}

#endif //NO_EVALUATOR_INITIALIZER

