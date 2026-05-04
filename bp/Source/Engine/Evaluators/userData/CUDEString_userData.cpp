#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories guserDataEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCUDEString_userData(guserDataEvaluatorFactoriesEngine, CUDEString_userData::kType_0x864BF971, CUDEString_userData::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CUDEString_userData::CUDEString_userData(CEvaluatorAllocator &allocator)
: IUserDataEvaluator()
//Start:CUDEString_userData_ctor:D6C231B8:33EA0106 *** Machine generated code - do not edit ***
{
   mIdentifier = std::string("_String");
//End:CUDEString_userData_ctor:D6C231B8 *** Machine generated code - do not edit *** $End$:CUDEString_userData_ctor:D6C231B8
}

CUDEString_userData::~CUDEString_userData()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

char const * CUDEString_userData::GetIdentifier() const
{
   return mIdentifier.c_str();
}

IUserDataEvaluator::EDataType CUDEString_userData::GetUserDataType() const
{
   return kDT_String;
}

void CUDEString_userData::GetUserData(CEvaluatorUpdateData const &updateData, void * pBufferOut) const
{
   char const ** ppStringOut = (char const **) pBufferOut;
   char const * pString = mValue.c_str();
   *ppStringOut = pString;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CUDEString_userData_body:0A2B1A0F:B0231F88 *** Machine generated code - do not edit ***
uint32 const CUDEString_userData::kType_0x864BF971 = 0x864BF971;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CUDEString_userData::GetType() const
{
   return kType_0x864BF971;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CUDEString_userData::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CUDEString_userData * pEval = new CUDEString_userData(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CUDEString_userData::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Identifier:   // 0x387380BA
         {
            mIdentifier = applyProperties.mStream.ReadString();
            break;
         }
         case kP_Value:   // 0xDCB67730
         {
            mValue = applyProperties.mStream.ReadString();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CUDEString_userData.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CUDEString_userData::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CUDEString_userData::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CUDEString_userData::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CUDEString_userData_body:0A2B1A0F *** Machine generated code - do not edit *** $End$:CUDEString_userData_body:0A2B1A0F
}
#endif //NO_EVALUATOR_INITIALIZER


