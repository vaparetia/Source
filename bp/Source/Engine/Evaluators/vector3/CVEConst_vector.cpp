#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVEConst_vector(gvector3EvaluatorFactoriesEngine, CVEConst_vector::kType_0xAE2FE58E, CVEConst_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVEConst_vector::CVEConst_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVEConst_vector_ctor:770C3507:2881582C *** Machine generated code - do not edit ***
 , mVector(CVector3::kConstructUninitialized)
{
   mVector = CVector3(0.0f, 0.0f, 0.0f);
//End:CVEConst_vector_ctor:770C3507 *** Machine generated code - do not edit *** $End$:CVEConst_vector_ctor:770C3507
}

CVEConst_vector::~CVEConst_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVEConst_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return mVector;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVEConst_vector_body:ABE51EB0:C8C475E0 *** Machine generated code - do not edit ***
uint32 const CVEConst_vector::kType_0xAE2FE58E = 0xAE2FE58E;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVEConst_vector::GetType() const
{
   return kType_0xAE2FE58E;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVEConst_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVEConst_vector * pEval = new CVEConst_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVEConst_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Vector:   // 0x1CC24D6D
         {
            mVector = CVector3(applyProperties.mStream);
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVEConst_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVEConst_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVEConst_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVEConst_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CVEConst_vector_body:ABE51EB0 *** Machine generated code - do not edit *** $End$:CVEConst_vector_body:ABE51EB0
}

#endif //NO_EVALUATOR_INITIALIZER

