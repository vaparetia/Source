#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEConst_color(gcolorEvaluatorFactoriesEngine, CCEConst_color::kType_0xF734AC54, CCEConst_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEConst_color::CCEConst_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCEConst_color_ctor:C9DB695F:04680EBD *** Machine generated code - do not edit ***
{
   mValue = CColorf(0.5f, 0.5f, 0.5f, 1.0f);
//End:CCEConst_color_ctor:C9DB695F *** Machine generated code - do not edit *** $End$:CCEConst_color_ctor:C9DB695F
}

CCEConst_color::~CCEConst_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEConst_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return mValue;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEConst_color_body:153242E8:BF5CD197 *** Machine generated code - do not edit ***
uint32 const CCEConst_color::kType_0xF734AC54 = 0xF734AC54;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEConst_color::GetType() const
{
   return kType_0xF734AC54;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEConst_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEConst_color * pEval = new CCEConst_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEConst_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
            mValue = CColorf(applyProperties.mStream);
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEConst_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEConst_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEConst_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEConst_color::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CCEConst_color_body:153242E8 *** Machine generated code - do not edit *** $End$:CCEConst_color_body:153242E8
}

#endif //NO_EVALUATOR_INITIALIZER

