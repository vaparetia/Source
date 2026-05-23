#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFETimeProviderEx_float(gfloatEvaluatorFactoriesEngine, CFETimeProviderEx_float::kType_0xE4F12DCB, CFETimeProviderEx_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFETimeProviderEx_float::CFETimeProviderEx_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFETimeProviderEx_float_ctor:095A4981:CE60CC93 *** Machine generated code - do not edit ***
{
   mType = kTPT_LocalTime;
   mScale = 1.0f;
   mWrap = 0.0f;
   mOffset = 0.0f;
//End:CFETimeProviderEx_float_ctor:095A4981 *** Machine generated code - do not edit *** $End$:CFETimeProviderEx_float_ctor:095A4981
}

CFETimeProviderEx_float::~CFETimeProviderEx_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFETimeProviderEx_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 time = 0;
   switch( mType )
   {
   case kTPT_LocalTime:
      time = updateData.mLocalTime;
      break;
   case kTPT_InstanceTime:
      time = updateData.mInstanceTime;
      break;
   case kTPT_UniverseTime:
      time = updateData.mUniverseTime;
      break;
   default:
      BPE_ASSERTA( "Invalid time type" );
      return 0.0f;
   }

   time *= mScale;
   if (mWrap > 0)
   {
      time = fmodf(time, mWrap);
   }
   time += mOffset;

   return time;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFETimeProviderEx_float_body:D5B36236:18DD4E97 *** Machine generated code - do not edit ***
uint32 const CFETimeProviderEx_float::kType_0xE4F12DCB = 0xE4F12DCB;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFETimeProviderEx_float::GetType() const
{
   return kType_0xE4F12DCB;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFETimeProviderEx_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFETimeProviderEx_float * pEval = new CFETimeProviderEx_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFETimeProviderEx_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_Scale:   // 0x2D870A80
         {
            mScale = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Wrap:   // 0x0C087FE3
         {
            mWrap = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Offset:   // 0x5EA6CFE6
         {
            mOffset = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFETimeProviderEx_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFETimeProviderEx_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFETimeProviderEx_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFETimeProviderEx_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFETimeProviderEx_float_body:D5B36236 *** Machine generated code - do not edit *** $End$:CFETimeProviderEx_float_body:D5B36236
}

#endif //NO_EVALUATOR_INITIALIZER

