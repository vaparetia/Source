#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFERandomVS_float(gfloatEvaluatorFactoriesEngine, CFERandomVS_float::kType_0x1B3E0C4F, CFERandomVS_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFERandomVS_float::CFERandomVS_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFERandomVS_float_ctor:D06BED08:A3089CA7 *** Machine generated code - do not edit ***
{
   mCenterValue = 0.5f;
   mRandomVariation = 0.5f;
//End:CFERandomVS_float_ctor:D06BED08 *** Machine generated code - do not edit *** $End$:CFERandomVS_float_ctor:D06BED08
}

CFERandomVS_float::~CFERandomVS_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFERandomVS_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const value = mCenterValue + ((updateData.mpRandom->RandomFloat() - 0.5f) * 2.0f * mRandomVariation);
   return value;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFERandomVS_float_body:0C82C6BF:3EB48C30 *** Machine generated code - do not edit ***
uint32 const CFERandomVS_float::kType_0x1B3E0C4F = 0x1B3E0C4F;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFERandomVS_float::GetType() const
{
   return kType_0x1B3E0C4F;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFERandomVS_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFERandomVS_float * pEval = new CFERandomVS_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFERandomVS_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFERandomVS_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFERandomVS_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFERandomVS_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFERandomVS_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFERandomVS_float_body:0C82C6BF *** Machine generated code - do not edit *** $End$:CFERandomVS_float_body:0C82C6BF
}

#endif //NO_EVALUATOR_INITIALIZER

