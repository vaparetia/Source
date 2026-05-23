#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEConst_float(gfloatEvaluatorFactoriesEngine, CFEConst_float::kType_0x58C7BA28, CFEConst_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFEConst_float::CFEConst_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFEConst_float_ctor:FA6C609A:321CAA01 *** Machine generated code - do not edit ***
{
   mValue = 0.0f;
//End:CFEConst_float_ctor:FA6C609A *** Machine generated code - do not edit *** $End$:CFEConst_float_ctor:FA6C609A

}

CFEConst_float::~CFEConst_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEConst_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return mValue;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEConst_float_body:26854B2D:47A876BA *** Machine generated code - do not edit ***
uint32 const CFEConst_float::kType_0x58C7BA28 = 0x58C7BA28;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEConst_float::GetType() const
{
   return kType_0x58C7BA28;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEConst_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEConst_float * pEval = new CFEConst_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEConst_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Value:   // 0xDCB67730
         {
            mValue = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEConst_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEConst_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEConst_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEConst_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFEConst_float_body:26854B2D *** Machine generated code - do not edit *** $End$:CFEConst_float_body:26854B2D
}

#endif //NO_EVALUATOR_INITIALIZER

