using System;
using System.Collections.Generic;
using System.Text;
using Tools;

namespace Tools.AssetSystem
{
   /// <summary>
   /// List of platforms for asset cooking.
   /// </summary>
   public class PlatformType
   {
      public enum EPlatform
      {
         kWin32,
         kPS3,
         kRVL,
         kX360, 
         kVita
      }

      static private int BuildPlatformCount()
      {
         return typeof( EPlatform ).GetFields().Length;
      }

      static private int skPlatformCount = BuildPlatformCount();

      public class PlatformListEnumerator : IEnumerator<EPlatform>
      {
         public PlatformListEnumerator( PlatformList list )
         {
            mList = list;
         }

         public EPlatform Current
         {
            get 
            { 
               if ( mCurrentBit == -1 || mCurrentBit == skPlatformCount )
               {
                  throw new ArgumentOutOfRangeException();
               }

               return (EPlatform) mCurrentBit;
            }
         }

         public void Dispose()
         {
         }

         object System.Collections.IEnumerator.Current
         {
            get 
            { 
               EPlatform platform = this.Current;

               return platform;
            }
         }

         public bool MoveNext()
         {
            if ( mCurrentBit == skPlatformCount )
            {
               return false;
            }

            for ( mCurrentBit = mCurrentBit + 1; mCurrentBit != skPlatformCount; ++mCurrentBit )
            {
               if ( 0 != ( mList.Bits & ( 1 << mCurrentBit ) ) )
               {
                  return true;
               }
            }

            return false;
         }

         public void Reset()
         {
            mCurrentBit = -1;
         }

         private PlatformList mList;
         private int mCurrentBit = -1;
      }

      public class PlatformList : IEnumerable<EPlatform>, ICloneable, IEquatable<PlatformList>, IComparable<PlatformList>
      {
         public IEnumerator<EPlatform> GetEnumerator()
         {
            return new PlatformListEnumerator( this );
         }

         System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
         {
            return new PlatformListEnumerator( this );
         }

         object ICloneable.Clone()
         {
            return this.Clone();
         }

         public PlatformList Clone()
         {
            PlatformList newList = new PlatformList();

            newList.mBits = mBits;
            return newList;
         }

         public int Count
         {
            get
            {
               int num = 0;
               for ( int i = 0; i < skPlatformCount; ++i )
               {
                  if ( ( mBits & ( 1 << i ) ) != 0 )
                  {
                     ++num;
                  }
               }

               return num;
            }
         }

         public bool Equals( PlatformList other )
         {
            return mBits == other.mBits;
         }

         public override bool Equals( object obj )
         {
            if ( obj is PlatformList )
            {
               return Equals( (PlatformList) obj );
            }
            else
            {
               return false;
            }
         }

         static public bool operator ==( PlatformList lhs, PlatformList rhs )
         {
            return lhs.Equals( rhs );
         }

         static public bool operator !=( PlatformList lhs, PlatformList rhs )
         {
            return !lhs.Equals( rhs );
         }

         public int CompareTo( PlatformList other )
         {
            return mBits - other.mBits;
         }

         public override int GetHashCode()
         {
            return Bits;
         }

         static public PlatformList FromPlatform( EPlatform platform )
         {
            PlatformList newList = new PlatformList();

            newList.Add( platform );
            return newList;
         }

         public EPlatform[] ToArray()
         {
            return new List<EPlatform>( this ).ToArray();
         }

         public void Add( EPlatform platform )
         {
            mBits |= 1 << ( (int) platform );
         }

         public void Remove( EPlatform platform )
         {
            mBits &= ~( 1 << ( (int) platform ) );
         }

         public void CombineWith( PlatformList other )
         {
            mBits |= other.mBits;
         }

         public void RemoveAllOf( PlatformList other )
         {
            mBits &= ~( other.mBits );
         }

         public int Bits
         {
            get { return mBits; }
         }

         public bool Empty
         {
            get { return mBits == 0; }
         }

         int mBits;
      }

      private string       mDescription;
      private string       mShortName;
      private string       mCPPDefine;
      private uint         mTypeId;
      private EPlatform    mPlatform;
      private bool         mIsBigEndian;

      public string     Description          { get { return mDescription; } }
      public string     ShortName            { get { return mShortName; } }
      public string     CPPDefine            { get { return mCPPDefine; } }
      public uint       TypeId               { get { return mTypeId; } }               /// Also used for directory name
      public string     TypeString           { get { return Tools.Common.Misc.GetStringFromFourCC(mTypeId); } }
      public EPlatform  Platform             { get { return mPlatform; } }
      public bool       IsBigEndian          { get { return mIsBigEndian; } }
      public bool       IsLittleEndian       { get { return !mIsBigEndian; } }

      public PlatformType(string description, string shortName, string cppDefine, string typeFourCC, EPlatform platform, bool isBigEndian)
      {
         mDescription = description;
         mShortName = shortName;
         mCPPDefine = cppDefine;
         mTypeId = Tools.Common.Misc.GetFourCCFromString(typeFourCC);
         mPlatform = platform;
         mIsBigEndian = isBigEndian;
      }

      static public readonly PlatformType[] sAssetPlatformTypes = new PlatformType[]
      {
         new PlatformType( "Win32",          "PC",    "WIN32", "_win",   PlatformType.EPlatform.kWin32,  false ),
         new PlatformType( "PlayStation3",   "PS3",   "PS3",   "_ps3",   PlatformType.EPlatform.kPS3,    true ),
         //new PlatformType( "Wii",            "Wii",   "RVL",   "_rvl",   PlatformType.EPlatform.kRVL,    true ),
         new PlatformType( "X360",           "X360",  "X360",  "_360",   PlatformType.EPlatform.kX360,   true ),
         new PlatformType( "PS Vita",        "Vita",  "VTA",   "_vta",   PlatformType.EPlatform.kVita,   true )
      };

      static public PlatformType GetPlatformTypeForEnum(PlatformType.EPlatform platformType)
      {
         foreach (PlatformType type in sAssetPlatformTypes)
         {
            if (type.Platform == platformType)
               return type;
         }

         return null;
      }

      static public PlatformType GetPlatformTypeForTypeId(uint typeId)
      {
         foreach (PlatformType type in sAssetPlatformTypes)
         {
            if (type.TypeId == typeId)
               return type;
         }

         return null;
      }

      static public PlatformType GetPlatformTypeForString(string typeString)
      {
         foreach (PlatformType type in sAssetPlatformTypes)
         {
            if (type.TypeString == typeString)
               return type;
         }

         return null;
      }

      static public PlatformType GetPlatformTypeForCPPDefine( string cppDefine )
      {
         foreach ( PlatformType type in sAssetPlatformTypes )
         {
            if ( type.CPPDefine == cppDefine )
            {
               return type;
            }
         }

         return null;
      }

      static PlatformList GenerateCookablePlatforms()
      {
         PlatformList platforms = new PlatformList();

         // The BPE_PLATFORM_COOK_EXCLUSIONS env var can be used to prevent asset cooking for specified platforms.
         // e.g. BPE_PLATFORM_COOK_EXCLUSIONS=_win;_360 would prevent Win32 and X360 from being cooked.
         // Intended to be customized on a USER basis based on available development HW
         string platformCookExclusions = System.Environment.GetEnvironmentVariable( "BPE_PLATFORM_COOK_EXCLUSIONS" );

         foreach ( PlatformType platformType in sAssetPlatformTypes )
         {
            // Check to see if we're excluding this platform
            if ((platformCookExclusions != null) && (platformCookExclusions.Contains(platformType.TypeString)))
            {
               // This platform has been explicitly excluded.
               continue;
            }
            platforms.Add(platformType.Platform);
         }

         return platforms;
      }

      public static PlatformList CookPlatforms
      {
         get { return sCookablePlatforms.Clone(); }
      }

      private static PlatformList sCookablePlatforms = GenerateCookablePlatforms();

   }

}
