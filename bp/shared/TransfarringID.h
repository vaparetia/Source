#pragma once
#include "Engine/Basics/BPEEnvironment.h"

enum ETransfarType
{
   kTT_Invalid,
   kTT_WiFi,
   kTT_Cloud,
   kTT_MaxTransfarTypes
};

namespace TransfarringDefines
{
   const int kHashLengthBytes = 16;
}

class TransfarringID
{
public:
   TransfarringID();
   virtual ~TransfarringID() { }

   // Creation
   void MakeNewInvalid();
   void MakeFromString( const char* src );
   void MakeNewWifi( bool bLocked );
   void MakeWifiFromUniqueID( const long long* uid );
   void MakeNewCloud( uint64 version );
   void Invalidate();
   void GenerateUniqueID();

   // Updating
   void UpdateCloudVersion(int version);

   // Writing to PARAM.SFO or MASTER.BIN
   void WriteToString(char* dst) const;

   bool IsWifiLocked() const;
   bool IsWifiLinked() const;
   bool HasOfflineWork() const;
   bool HasValidUniqueID() const;

   bool DoUniqueIDsMatch(const unsigned char* otherID) const;
   bool DoVersionIDsMatch(const unsigned char* otherID) const;
   bool DoPSNAccountsMatch(const unsigned char* otherID) const;

   bool operator==(const TransfarringID& other) const
   {
      return m_UniqueID[0] == other.m_UniqueID[0] && m_UniqueID[1] == other.m_UniqueID[1];
   }

   static void CreateHash(unsigned char * const dst, const unsigned char * data, unsigned int dataSize);

   ETransfarType m_Type; // Type of ID

   // NOTE THAT THESE ARE NOT STRINGS!!!! They're 16 bytes of MD5 hash data, don't use strcmp/strlen/strcpy
   long long m_UniqueID[2]; // Both WiFi and cloud
   long long m_VersionID[2]; // Cloud only; if there's a mismatch between the local version and the server version, it needs to sync
   long long m_PSNAccountName[2]; // Cloud only; MD5 hash of PSN account name
   bool m_HasOfflineWork;
   bool m_IsLocked;

private:
   const char* ConvertHexStringToNumber(unsigned char* byteArray, const char* stringSource);
};
