#pragma once

public ref class CZLib
{
public:
   static System::Int32 Compress(cli::array<System::Byte> ^ inDestination, System::Int64 % inoutDestinationLength,
                                 cli::array<System::Byte> ^ inSource, System::Int64 inSourceLength);

   static System::Int32 Uncompress(cli::array<System::Byte> ^ inDestination, System::Int64 % inoutDestinationLength,
                                   cli::array<System::Byte> ^ inSource, System::Int64 inSourceLength);
};
