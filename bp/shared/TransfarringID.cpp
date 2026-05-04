#include "TransfarringID.h"
#include "Engine/Stdafx.h"
#include "MGS_Common.h"
#include "BP_Network.h"
#include "BP_EndianSupport.h"

#include <sstream>

#include "BP_MD5.h"

// Platform-specific includes
#if defined(BP_VITA)
#include "TransfarringVTA.h"
#endif

#if defined(BP_PS3)
#include <sys/sys_time.h>
#include "Transfarring_PS3.h"
#endif

static const char skTransfarringStringBase[] = "Transfarring ID: ";
static const char skInvalidDataChar = 'U';
static const char skWifiDataChar = 'W';
static const char skCloudDataChar = 'C';
static const char skCloudOfflineDataChar = 'O';
static const char skNoCloudOfflineDataChar = 'N';
static const char skWifiLockedChar = 'L';
static const char skWifiLinkedChar = 'I';

// TransfarringID
TransfarringID::TransfarringID()
: m_Type( kTT_Invalid )
, m_HasOfflineWork( false )
, m_IsLocked( false )
{
   memset(m_UniqueID, 0, TransfarringDefines::kHashLengthBytes);
   memset(m_VersionID, 0, TransfarringDefines::kHashLengthBytes);
   memset(m_PSNAccountName, 0, TransfarringDefines::kHashLengthBytes);
}

void TransfarringID::MakeNewInvalid()
{
   m_Type = kTT_Invalid;
   GenerateUniqueID();
}

void TransfarringID::MakeFromString(const char* src)
{
   // "Transfarring ID: W 0xWHATEVER"
   src += strlen(skTransfarringStringBase);

   if (*src == skWifiDataChar)
   {
      m_Type = kTT_WiFi;
      src += 2;

      // Rest of the string is the Unique ID
      src = ConvertHexStringToNumber((unsigned char*)m_UniqueID, src);

      if( *src )
      {
         // Skip whitespace
         src += 1;

         if( *src == skWifiLockedChar )
         {
            m_IsLocked = true;
         }
         else if( *src == skWifiLinkedChar )
         {
            m_IsLocked = false;
         }
         else
         {
            // Old saves didn't have any data
#if BP_PS3
            m_IsLocked = true;
#else
            m_IsLocked = false;
#endif
         }
      }
      else
      {
         // Old saves didn't have any data
#if BP_PS3
         m_IsLocked = true;
#else
         m_IsLocked = false;
#endif
      }
   }
   else if (*src == skCloudDataChar)
   {
      m_Type = kTT_Cloud;
      src += 2;

      // Unique ID
      src = ConvertHexStringToNumber((unsigned char*)m_UniqueID, src);

      // Version hash
      src += 1; // Skip newline
      if (*src == skCloudOfflineDataChar)
      {
         m_HasOfflineWork = true;
      }
      else if (*src == skNoCloudOfflineDataChar)
      {
         m_HasOfflineWork = false;
      }
      else
      {
         BP_BREAK; // It has to be one of these, we screwed something up if it's not
      }
      src += 2; // cloud character and a space

      src = ConvertHexStringToNumber((unsigned char*)m_VersionID, src);

      // PSN account name hash
      src += 1; // Skip newline

      // Rest of the string is the PSN account hash
      BPE_ASSERT(strlen(src) == 16 * 2, "TransfarringID::MakeFromString - Buffer overrun");

      src = ConvertHexStringToNumber((unsigned char*)m_PSNAccountName, src);
   }
   else if( *src == skInvalidDataChar )
   {
      m_Type = kTT_Invalid;
      src += 2;

      // Rest of the string is the Unique ID
      src = ConvertHexStringToNumber((unsigned char*)m_UniqueID, src);
   }

   // Clear out IDs of old saves that accidentally saved zeroed IDs
   if (m_Type != kTT_Invalid && m_UniqueID[0] == 0 && m_UniqueID[1] == 0)
   {
      m_Type = kTT_Invalid;
   }
}

void TransfarringID::MakeNewWifi( bool bLocked )
{
   m_Type = kTT_WiFi;

   if( !HasValidUniqueID() )
   {
      GenerateUniqueID();
   }

   m_IsLocked = bLocked;
}

void TransfarringID::MakeWifiFromUniqueID(const long long* uid)
{
   m_Type = kTT_WiFi;
   memcpy(m_UniqueID, uid, TransfarringDefines::kHashLengthBytes);
}

void TransfarringID::MakeNewCloud( uint64 version )
{
   m_Type = kTT_Cloud;

   BP_LE_Swap8Bytes_Inp(&version);

   if( !HasValidUniqueID() )
   {
      GenerateUniqueID();
   }
   
#if defined(BP_VITA) || defined(BP_PS3)
   // Create the MD5 hash from the version number. 
   // We don't have to store the version number itself, just the hash. 
   // If there's no offline work, you can always sync safely.
   // If there's offline work, and the version hashes match, you can upload safely
   // If there's offline work, and the versions don't match, you'll be prompted to resolve the conflict.
   CreateHash((unsigned char*)m_VersionID, (unsigned char*)&version, sizeof(version));
   
   // Create the MD5 hash from the PSN account name
   CTransfarringManager::GetPSNAccountHash( true, (unsigned char*)m_PSNAccountName, (unsigned char*)m_UniqueID );

#else
   BP_BREAK;
#endif
}

void TransfarringID::Invalidate()
{
   m_Type = kTT_Invalid;

   // Keep the Unique ID no matter how this ID changes

   memset(m_VersionID, 0, TransfarringDefines::kHashLengthBytes);
   memset(m_PSNAccountName, 0, TransfarringDefines::kHashLengthBytes);

   m_HasOfflineWork = false;
   m_IsLocked = false;
}

void TransfarringID::WriteToString(char* dst) const
{
   char* sizeChecker = dst;

   strcpy(dst, skTransfarringStringBase);
   dst += strlen(skTransfarringStringBase);

   if( m_Type == kTT_Invalid )
   {
      dst[0] = skInvalidDataChar;
      dst[1] = ' ';
      dst += 2;

      const unsigned char* byteArray = (const unsigned char*)m_UniqueID;
      for (int i=0; i < TransfarringDefines::kHashLengthBytes; i++)
      {
         sprintf(dst + i * 2, "%02x", byteArray[i]);
      }
      dst += TransfarringDefines::kHashLengthBytes * 2; // 2 bytes per hex character

      *dst = 0;
   }
   else if (m_Type == kTT_Cloud)
   {
      dst[0] = skCloudDataChar;
      dst[1] = ' ';
      dst += 2;

      // Write unique ID
      const unsigned char* byteArray = (const unsigned char*)m_UniqueID;
      for (int i=0; i < TransfarringDefines::kHashLengthBytes; i++)
      {
         sprintf(dst + i * 2, "%02x", byteArray[i]);
      }
      dst += TransfarringDefines::kHashLengthBytes * 2; // 2 bytes per hex character

      // Write cloud version; O indicates there is offline work, N indicates no offline work
      if (HasOfflineWork())
      {
         dst[0] = '\n';
         dst[1] = skCloudOfflineDataChar;
         dst[2] = ' ';
         dst += 3;
      }
      else
      {
         dst[0] = '\n';
         dst[1] = skNoCloudOfflineDataChar;
         dst[2] = ' ';
         dst += 3;
      }

      // Write version hash
      byteArray = (const unsigned char*)m_VersionID;
      for (int i=0; i < TransfarringDefines::kHashLengthBytes; i++)
      {
         sprintf(dst + i * 2, "%02x", byteArray[i]);
      }
      dst += TransfarringDefines::kHashLengthBytes * 2; // 2 bytes per hex character

      dst[0] = '\n';
      dst += 1;

      // Write PSN account hash
      byteArray = (const unsigned char*)m_PSNAccountName;
      for (int i=0; i < TransfarringDefines::kHashLengthBytes; i++)
      {
         sprintf(dst + i * 2, "%02x", byteArray[i]);
      }
      dst += TransfarringDefines::kHashLengthBytes * 2; // 2 bytes per hex character

      *dst = 0; // Null terminator, just in case
   }
   else if (m_Type == kTT_WiFi)
   {
      dst[0] = skWifiDataChar;
      dst[1] = ' ';
      dst += 2;

      const unsigned char* byteArray = (const unsigned char*)m_UniqueID;
      for (int i=0; i < TransfarringDefines::kHashLengthBytes; i++)
      {
         sprintf(dst + i * 2, "%02x", byteArray[i]);
      }
      dst += TransfarringDefines::kHashLengthBytes * 2; // 2 bytes per hex character

      dst[0] = ' ';
      dst[1] = m_IsLocked ? skWifiLockedChar : skWifiLinkedChar;
      dst[2] = 0; // Null terminator, just in case
   }

   BPE_ASSERT(dst - sizeChecker < (1024 - 32), "TransfarringID::WriteToString buffer overrun!");
}

bool TransfarringID::IsWifiLocked() const
{
   return m_Type == kTT_WiFi && HasValidUniqueID() && m_IsLocked;
}

bool TransfarringID::IsWifiLinked() const
{
   return m_Type == kTT_WiFi && HasValidUniqueID() && !m_IsLocked;
}

bool TransfarringID::HasOfflineWork() const
{
   return m_HasOfflineWork;
}

bool TransfarringID::HasValidUniqueID() const
{
   return m_UniqueID[0] != 0 || m_UniqueID[1] != 0;
}

bool TransfarringID::DoUniqueIDsMatch(const unsigned char* otherID) const
{
   return memcmp(m_UniqueID, otherID, TransfarringDefines::kHashLengthBytes) == 0;
}

bool TransfarringID::DoVersionIDsMatch(const unsigned char* otherID) const
{
   return memcmp(m_VersionID, otherID, TransfarringDefines::kHashLengthBytes) == 0;
}

bool TransfarringID::DoPSNAccountsMatch(const unsigned char* otherID) const
{
   return memcmp(m_PSNAccountName, otherID, TransfarringDefines::kHashLengthBytes) == 0;
}

void TransfarringID::CreateHash(unsigned char * const dst, const unsigned char* data, unsigned int dataSize)
{
   BP_CreateMD5Hash( dst, data, dataSize );
}

void TransfarringID::GenerateUniqueID()
{
#if defined (BP_PS3)
   // Create a buffer consisting of the current second since 1970 + the system time.
   // We are no longer using the PSID.

   sys_time_sec_t sec;
   sys_time_nsec_t nsec;
   sys_time_get_current_time(&sec, &nsec);

   system_time_t const sysTime = sys_time_get_system_time();

   unsigned char hashBuffer[sizeof( sys_time_sec_t ) + sizeof( sys_time_nsec_t ) + sizeof(system_time_t)];
   CTransfarringManager::Instance()->GetConsoleOpenPSID( hashBuffer );

   memcpy( hashBuffer, &sec, sizeof( sys_time_sec_t ) );
   memcpy( hashBuffer + sizeof( sys_time_sec_t ), &nsec, sizeof( sys_time_nsec_t ) );
   memcpy( hashBuffer + sizeof( sys_time_sec_t ) + sizeof( sys_time_nsec_t ), &sysTime, sizeof( system_time_t ) );

   // Create the MD5 hash
   CreateHash( (unsigned char*)m_UniqueID, hashBuffer, sizeof( hashBuffer ) );
#elif defined (BP_VITA)
   // Create a buffer consisting of the OpenPSID and the current second since 1970
   time_t const curTime = time( NULL );
   
   SceRtcTick curTick;
   sceRtcGetCurrentTick( &curTick );

   SceUInt64 const curProcTime = sceKernelGetProcessTimeWide();

   unsigned char hashBuffer[sizeof( time_t ) + sizeof( SceRtcTick ) + sizeof( SceUInt64 )];

   memcpy( hashBuffer, &curTime, sizeof( time_t ) );
   memcpy( hashBuffer + sizeof( time_t ), &curTick, sizeof( SceRtcTick ) );
   memcpy( hashBuffer + sizeof( time_t ) + sizeof( SceRtcTick ), &curProcTime, sizeof( SceUInt64 ) );

   // Create the MD5 hash
   CreateHash((unsigned char*)m_UniqueID, hashBuffer, sizeof( hashBuffer ) );
#endif
}

const char* TransfarringID::ConvertHexStringToNumber(unsigned char* byteArray, const char* stringSource)
{
   int i=0;
   unsigned char temp[3];
   memset(temp, 0, sizeof(temp));

   while (*stringSource != 0 && *stringSource != '\n' && *stringSource != ' ')
   {
      std::stringstream ss;
      memcpy(temp, stringSource, 2);
      ss << std::hex << temp;
      unsigned short shortCast;
      ss >> shortCast;
      byteArray[i] = (unsigned char)shortCast; // 2 hex numbers, guaranteed to fit in 1 byte
      i++;
      stringSource += 2;
   }

   return stringSource;
}