#include "StdAfx.h"

#include "ICGLdrShader.h"
#include "Engine/GameObjectSystem/CGameObjectRegisteredComponentGroupObjectFactories.h"

// Code gen will actually generate an include for this, but as we're dealing with a single case, we'll just stick the extern here.
extern CGameObjectRegisteredComponentGroupObjectFactories gsComponentGroupObjectFactoriesShader;

//Start:ICGLdrShader_body:2D3A68BF:34389869 *** Machine generated code - do not edit ***
#include "CCPLdrShaderDefaultShader.h"

// Register factory
BPE_FORCE_REFERENCE CGameObjectRegisterComponentGroupObjectFactory sRegisterFactoryICGLdrShader( gsComponentGroupObjectFactoriesShader, ICGLdrShader::kComponentGroupType, ICGLdrShader::BuildComponentProperties, NULL);

//----------------------------------------------------------------------------

ICGLdrShader::ICGLdrShader()
{
}

//----------------------------------------------------------------------------

ICGLdrShader::~ICGLdrShader()
{
}

//ICGLdrShader::PostLoadUpdate()
//{
//}

//----------------------------------------------------------------------------

char const * ICGLdrShader::GetComponentGroupName(void) const
{
   return "Shader";
}

//----------------------------------------------------------------------------

uint32 ICGLdrShader::GetComponentGroupType(void) const
{
   return kComponentGroupType;
}

//----------------------------------------------------------------------------

CGameObjectComponentProperties * ICGLdrShader::BuildComponentProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator * pAllocator)
{
   uint32 const fourCC = applyProperties.mStream.ReadUint32();
   uint16 size = applyProperties.mStream.ReadUint16();
   switch (fourCC)
   {
      case CCPLdrShaderDefaultShader::kComponentPropertiesType:   // 0x283F6879 / DefaultShader - (DefaultShader)
      {
         CCPLdrShaderDefaultShader * pComponentProperties = new CCPLdrShaderDefaultShader();
         pComponentProperties->ApplyProperties(applyProperties, pAllocator ? *pAllocator : pComponentProperties->mEvaluatorAllocator);
         pComponentProperties->PostLoadUpdate();
         return pComponentProperties;
      }
      case 0:   // 'None' / Empty component.
         applyProperties.mStream.Get( NULL, size );
         return NULL;
      default:
         property_load_error_printf( "Unknown component 0x%08x in component loader ICGLdrShader.\n", fourCC );
         applyProperties.mStream.Get( NULL, size );
         return NULL;
   }
}

//End:ICGLdrShader_body:2D3A68BF *** Machine generated code - do not edit *** $End$:ICGLdrShader_body:2D3A68BF



