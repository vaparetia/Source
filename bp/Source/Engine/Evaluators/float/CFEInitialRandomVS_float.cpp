#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEInitialRandomVS_float(gfloatEvaluatorFactoriesEngine, CFEInitialRandomVS_float::kType_0x060AAB0D, CFEInitialRandomVS_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEInitialRandomVS_float::CFEInitialRandomVS_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mInitialValue(allocator.AllocFloat())
//Start:CFEInitialRandomVS_float_ctor:D1FF944B:A3089CA7 *** Machine generated code - do not edit ***
{
   mCenterValue = 0.5f;
   mRandomVariation = 0.5f;
//End:CFEInitialRandomVS_float_ctor:D1FF944B *** Machine generated code - do not edit *** $End$:CFEInitialRandomVS_float_ctor:D1FF944B
}

CFEInitialRandomVS_float::~CFEInitialRandomVS_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEInitialRandomVS_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const value = updateData.mDataBlock.GetValue(mInitialValue);
   return value;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEInitialRandomVS_float_body:0D16BFFC:8AD22113 *** Machine generated code - do not edit ***
uint32 const CFEInitialRandomVS_float::kType_0x060AAB0D = 0x060AAB0D;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEInitialRandomVS_float::GetType() const
{
   return kType_0x060AAB0D;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEInitialRandomVS_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEInitialRandomVS_float * pEval = new CFEInitialRandomVS_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEInitialRandomVS_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            mCenterValue = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_RandomVariation:   // 0xDE09F15B
         {
            mRandomVariation = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEInitialRandomVS_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEInitialRandomVS_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEInitialRandomVS_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEInitialRandomVS_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFEInitialRandomVS_float_body:0D16BFFC *** Machine generated code - do not edit *** $End$:CFEInitialRandomVS_float_body:0D16BFFC
   real32 const value = mCenterValue + ((updateData.mpRandom->RandomFloat() - 0.5f) * 2.0f * mRandomVariation);

   updateData.mDataBlock.SetValue(mInitialValue, value);
}

#endif //NO_EVALUATOR_INITIALIZER

