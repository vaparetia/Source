#pragma once

#include "Renderer/Base/BPERendererAPI.h"
#include "../CGameObjectComponentPropertiesShader.h"

class CGameObjectComponent;

//Start:ICGLdrShader_declaration:9DDFFBFD:71885F00 *** Machine generated code - do not edit ***
class RENDERER_API ICGLdrShader : public CGameObjectComponentPropertiesShader
{
public:
   ICGLdrShader();
   virtual ~ICGLdrShader();

   static CGameObjectComponentProperties *         BuildComponentProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator * pAllocator = NULL);
   //virtual void	PostLoadUpdate();
   virtual char const *                            GetComponentGroupName(void) const;
   virtual uint32                                  GetComponentGroupType(void) const;

   static const uint32                             kComponentGroupType = 0xF85F9B1E;

};

//End:ICGLdrShader_declaration:9DDFFBFD *** Machine generated code - do not edit *** $End$:ICGLdrShader_declaration:9DDFFBFD

