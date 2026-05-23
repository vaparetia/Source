#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFETimeProvider_float(gfloatEvaluatorFactoriesEngine, CFETimeProvider_float::kType_0x275039A8, CFETimeProvider_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFETimeProvider_float::CFETimeProvider_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFETimeProvider_float_ctor:D4A14988:95C37251 *** Machine generated code - do not edit ***
{
   mType = kTPT_LocalTime;
//End:CFETimeProvider_float_ctor:D4A14988 *** Machine generated code - do not edit *** $End$:CFETimeProvider_float_ctor:D4A14988

}

CFETimeProvider_float::~CFETimeProvider_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFETimeProvider_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   switch( mType )
   {
   case kTPT_LocalTime:
      return updateData.mLocalTime;
   case kTPT_InstanceTime:
      return updateData.mInstanceTime;
   case kTPT_UniverseTime:
      return updateData.mUniverseTime;
   default:
      BPE_ASSERTA( "Invalid time type" );
      return 0.0f;
   }
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFETimeProvider_float_body:0848623F:D65E0B55 *** Machine generated code - do not edit ***
uint32 const CFETimeProvider_float::kType_0x275039A8 = 0x275039A8;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFETimeProvider_float::GetType() const
{
   return kType_0x275039A8;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFETimeProvider_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFETimeProvider_float * pEval = new CFETimeProvider_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFETimeProvider_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Type:   // 0x2CECF817
         {
            uint32 const enumFourCC = applyProperties.mStream.ReadUint32();
            switch (enumFourCC)
            {
               case 0x9DD50323:   // kTPT_LocalTime - Local Time
                  mType = kTPT_LocalTime;
                  break;
               case 0x64ED1294:   // kTPT_InstanceTime - Instance Time
                  mType = kTPT_InstanceTime;
                  break;
               case 0x54677195:   // kTPT_UniverseTime - Universe Time
                  mType = kTPT_UniverseTime;
                  break;
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFETimeProvider_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFETimeProvider_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFETimeProvider_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFETimeProvider_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFETimeProvider_float_body:0848623F *** Machine generated code - do not edit *** $End$:CFETimeProvider_float_body:0848623F

}

#endif //NO_EVALUATOR_INITIALIZER

