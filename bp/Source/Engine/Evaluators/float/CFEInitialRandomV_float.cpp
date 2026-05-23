#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialRandomV_float(gfloatEvaluatorFactoriesEngine, CFEInitialRandomV_float::kType_0xF622EFC3, CFEInitialRandomV_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialRandomV_float::CFEInitialRandomV_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialRandomV_float_ctor:1255DF77:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEInitialRandomV_float_ctor:1255DF77 *** Machine generated code - do not edit *** $End$:CFEInitialRandomV_float_ctor:1255DF77
}

CFEInitialRandomV_float::~CFEInitialRandomV_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialRandomV_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const value = updateData.mDataBlock.GetValue(mInitialValue);
   return value;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialRandomV_float_body:CEBCF4C0:C08A833F *** Machine generated code - do not edit ***
uint32 const CFEInitialRandomV_float::kType_0xF622EFC3 = 0xF622EFC3;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialRandomV_float::GetType() const
{
   return kType_0xF622EFC3;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialRandomV_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialRandomV_float * pEval = new CFEInitialRandomV_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialRandomV_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_CenterValue:   // 0x2AA88BCF
         {
            if (!applyProperties.UpdateOnly())
            {
               mCenterValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mCenterValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_RandomVariation:   // 0xDE09F15B
         {
            if (!applyProperties.UpdateOnly())
            {
               mRandomVariation.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mRandomVariation.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialRandomV_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialRandomV_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mCenterValue)
   {
      uint32 const evalPtr = mCenterValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mCenterValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mRandomVariation)
   {
      uint32 const evalPtr = mRandomVariation->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mRandomVariation) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialRandomV_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mCenterValue) mCenterValue->PostLoadUpdate();
   if (mRandomVariation) mRandomVariation->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialRandomV_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mCenterValue.get()) mCenterValue->Initialize(updateData);
   if (mRandomVariation.get()) mRandomVariation->Initialize(updateData);
//End:CFEInitialRandomV_float_body:CEBCF4C0 *** Machine generated code - do not edit *** $End$:CFEInitialRandomV_float_body:CEBCF4C0

   real32 const centerValue = mCenterValue->GetValue(updateData);
   real32 const randomVariation = mRandomVariation->GetValue(updateData);
   real32 const value = centerValue + ((updateData.mpRandom->RandomFloat() - 0.5f) * 2.0f * randomVariation);

   updateData.mDataBlock.SetValue(mInitialValue, value);
}

#endif //NO_EVALUATOR_INITIALIZER

