#include "../Engine/Mechanics/zlib/zlib.h"
#include "ZLib.h"

using namespace cli;

System::Int32 CZLib::Compress(array<System::Byte> ^ inDestination, System::Int64 % inoutDestinationLength, array<System::Byte> ^ inSource, System::Int64 inSourceLength)
{
   pin_ptr<System::Byte> pDst = &inDestination[0];
   pin_ptr<System::Byte> pSrc = &inSource[0];
   pin_ptr<System::Int64> pDstLen = &inoutDestinationLength;

   return compress((Bytef*)pDst, (uLongf*)pDstLen, (const Bytef*)pSrc, (uLongf)inSourceLength);
}

System::Int32 CZLib::Uncompress(array<System::Byte> ^ inDestination, System::Int64 % inoutDestinationLength, array<System::Byte> ^ inSource, System::Int64 inSourceLength)
{
   pin_ptr<System::Byte> pDst = &inDestination[0];
   pin_ptr<System::Byte> pSrc = &inSource[0];
   pin_ptr<System::Int64> pDstLen = &inoutDestinationLength;

   return uncompress((Bytef*)pDst, (uLongf *)pDstLen, (const Bytef*)pSrc, (uLongf)inSourceLength);
}