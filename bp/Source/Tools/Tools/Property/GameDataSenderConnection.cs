using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Xml;
using System.Xml.XPath;
using System.Timers;

using Tools;
using Tools.Property;
using Tools.Property.Source;
using Tools.Property.Cook;
using Tools.Property.Build;
using Tools.Property.Misc;
using Tools.Common;

/// Used to send lua strings and live property updates to GameConnection (and then onto game).
namespace Tools.Property.Misc
{
   public class GameDataSenderConnection
   {
      public TcpClient                                      mConnection;
      Int32                                                 mPort;
      DateTime                                              mLastConnectionAttempt;
      public static readonly Int32                          mDefaultGameDataPort = 32002;
      
      System.Timers.Timer                                   mSendTimer;
      Dictionary<Guid, XmlDocument>                         mCachedSendData;
      public CookProperties.CookInfo.CustomPropertyCooker   mCustomPropertyCookers; // Assign custom cookers to this.
      public CookProperties.CookInfo.ComponentPropertiesModifier mComponentGroupPropertiesModifier;

      /// <summary>Returns true if connected.</summary>
      bool                    Connected { get { return ((mConnection != null) && mConnection.Connected); } }

      public GameDataSenderConnection(Int32 port)
      {
         mSendTimer = new System.Timers.Timer();
         mSendTimer.Interval = 33;
         mSendTimer.AutoReset = false; // One shot timer
         mSendTimer.Elapsed += new ElapsedEventHandler(OnSendTimerElapsed);

         mCachedSendData = new Dictionary<Guid, XmlDocument>();

         mPort = port;
         Connect();
      }

      void ForceConnect()
      {
         try
         {
            mConnection = new TcpClient( "127.0.0.1", mPort );
            mConnection.NoDelay = true;
         }
         catch ( SocketException e )
         {
            Debug.WriteLine( e.ToString() );
         }
         finally
         {
            mLastConnectionAttempt = DateTime.Now;
         }
      }

      void Connect()
      {
         if (Connected) return;

         if (mLastConnectionAttempt.AddSeconds(5.0f) > DateTime.Now)
         {
            // tried to connect less than 5s ago
            return;
         }

         ForceConnect();
      }

      void WritetoConnectionWithRetry( byte[] buffer, int offset, int size )
      {
         // This function will write, and if during the write the connection dies,
         // it will reconnect and write.

         try
         {
            mConnection.GetStream().Write( buffer, offset, size );
         }
         catch (System.IO.IOException )
         {
            mConnection.Close();
            mConnection = null;

            ForceConnect();

            if ( Connected )
            {
               mConnection.GetStream().Write( buffer, offset, size );
            }
         }
      }

      public void SendGuid(Guid guid)
      {
         // Attempt connection
         Connect();
         if (!Connected)
         {
            // Not connected, skip
            return;
         }
         // Build full chunk of data before sending it down network connection as GameConnection.exe expects an entire data chunk to be available
         MemoryStream tempStream = new MemoryStream();
         BinaryWriterBigEndian tempWriter = new BinaryWriterBigEndian(tempStream);
         tempWriter.Write(guid.ToByteArray());
         tempWriter.Flush();

         MemoryStream fullPacketStream = new MemoryStream();
         BinaryWriterBigEndian outputStream = new BinaryWriterBigEndian(fullPacketStream);
         outputStream.Write(Tools.Common.Misc.GetFourCCFromString("GUID"));
         outputStream.Write((int)tempStream.Length);
         outputStream.Write(tempStream.GetBuffer(), 0, (int)tempStream.Length);
         outputStream.Flush();

         // Now send over socket
         WritetoConnectionWithRetry(fullPacketStream.GetBuffer(), 0, (int)fullPacketStream.Length);
      }

      public void SendLuaString(string luaString)
      {
         // Attempt connection
         Connect();
         if (!Connected)
         {
            // Not connected, skip
            return;
         }
         // Build full chunk of data before sending it down network connection as GameConnection.exe expects an entire data chunk to be available
         MemoryStream tempStream = new MemoryStream();
         BinaryWriterBigEndian tempWriter = new BinaryWriterBigEndian(tempStream);
         tempWriter.Write(luaString);
         tempWriter.Flush();
         
         MemoryStream fullPacketStream = new MemoryStream();
         BinaryWriterBigEndian outputStream = new BinaryWriterBigEndian(fullPacketStream);
         outputStream.Write(Tools.Common.Misc.GetFourCCFromString("LUAS"));
         outputStream.Write((int) tempStream.Length);
         outputStream.Write(tempStream.GetBuffer(), 0, (int) tempStream.Length);
         outputStream.Flush();
         
         // Now send over socket
         WritetoConnectionWithRetry( fullPacketStream.GetBuffer(), 0, (int) fullPacketStream.Length );
      }

      public void SendEditorPacket(uint typeId, MemoryStream dataStream)
      {
         // Attempt connection
         Connect();

         if (!Connected)
         {
            // Not connected, skip
            return;
         }

         MemoryStream fullPacketStream = new MemoryStream();
         BinaryWriterBigEndian outputStream = new BinaryWriterBigEndian(fullPacketStream);
         outputStream.Write(Tools.Common.Misc.GetFourCCFromString("EDIT"));

         // the packet length is 4 bytes extra to account for the type id that is not written out as part of the data stream.
         int packetLength = (int)dataStream.Length + 4; 
         
         outputStream.Write(packetLength);
         outputStream.Write(typeId);
         outputStream.Write(dataStream.GetBuffer(), 0, (int)dataStream.Length);

         outputStream.Flush();

         // Now send over socket
         WritetoConnectionWithRetry( fullPacketStream.GetBuffer(), 0, (int) fullPacketStream.Length );
      }

      /// <summary>
      /// This function ungroups properties, finds the components associated with the modified property,
      /// then sends the modified properties over the data connection.
      /// Normally used with properties value changed callback.
      /// </summary>
      /// <param name="modifedProperty"></param>
      /// <param name="groupedObjects"></param>
      public void SendModifiedComponentProperties( Data.Property modifiedProperty, List<XmlElement> groupedObjects )
      {
         // Attempt connection
         Connect();
         if (!Connected)
         {
            // Not connected, skip
            return;
         }

         if( modifiedProperty.SubType == Helpers.kProperty_SubType_asset )
         {
            string asset = modifiedProperty.ValueTextNullable;
            if( !string.IsNullOrEmpty(asset) )
            {
               Tools.AssetSystem.Manager assetManager = new Tools.AssetSystem.Manager();
               
               if( AssetSystem.Helper.NeedsCookingOnAnyPlatform(asset, assetManager, null) == Tools.AssetSystem.Helper.ENeedsCooking.Yes )
               {
                  List<string> assets = new List<string>();
                  assets.Add(asset);

                  AssetSystem.Helper.CookAssetsWin32(assets, Tools.AssetSystem.Helper.ECookAssetsMode.DisplayGUI|Tools.AssetSystem.Helper.ECookAssetsMode.WaitForFinish);
               }
            }
         }

         // Make full copy of grouped xml
         List<XmlElement> ungroupedProperties = new List<XmlElement>();
         foreach ( XmlElement nav in groupedObjects )
         {
            XmlDocument doc = XmlNodeHelpers.CreateDocumentFromNode( nav );
            XmlElement child = Tools.Common.XmlNodeHelpers.GetDocumentRootElement( doc );
            ungroupedProperties.Add( child );
         }

         GroupProperties.UngroupProperties( modifiedProperty.Element.OwnerDocument, ungroupedProperties, GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId );
         string modifiedPropertyXPath = GroupProperties.BuildPropertyXPathForValue( modifiedProperty.Element, 
                                                                                    GroupProperties.EGroupOptions.kIgnoreTopName_IgnoreTopId, 
                                                                                    GroupProperties.ELayoutElementProcessing.kKeepLayoutElements);

         // Add bitfield properties after the properties have been ungrouped from the property editor.
         foreach( XmlElement ungroupedPropertyContainer in ungroupedProperties )
         {
            Helpers.AddBitFieldProperties(ungroupedPropertyContainer);
         }

         foreach(XmlElement nav in ungroupedProperties)
         {
            // Find component containing modified property
            XmlElement topComponentGroupNav = null;
            XmlElement propertyNav = nav.SelectSingleNode(modifiedPropertyXPath) as XmlElement;
            
            if ( propertyNav == null )
            {
               continue;
            }
            
            // Now move up nodes until we find a component
            while ( propertyNav != null )
            {
               if ( propertyNav.Name == Helpers.kElement_ComponentGroup )
               {
                  // Found a component group, keep going, as we may have components within components
                  topComponentGroupNav = propertyNav;
               }

               propertyNav = propertyNav.ParentNode as XmlElement;
            }

            if (topComponentGroupNav != null)
            {
               // If we found a component, send the properties
               SendModifiedComponentProperties( topComponentGroupNav );
            }

         }
      }

      public void SendModifiedComponentProperties(XmlElement topComponentGroupNav)
      {
         XmlDocument componentGroupDoc = XmlNodeHelpers.CreateDocumentFromNode( topComponentGroupNav );

         Guid componentGroupGuid = Helpers.GetInternalEditorIdGuid(topComponentGroupNav);

         lock( mCachedSendData )
         {
            mCachedSendData.Remove(componentGroupGuid);
            mCachedSendData.Add(componentGroupGuid, componentGroupDoc);
         }
         mSendTimer.Start();
      }

      void OnSendTimerElapsed(object sender, ElapsedEventArgs e)
      {
         lock( mCachedSendData )
         {
            if( mCachedSendData.Count == 0 )
               return;

            Tools.AssetSystem.Manager assetManager = new Tools.AssetSystem.Manager();

            foreach (KeyValuePair<Guid, XmlDocument> componentGroupPair in mCachedSendData)
            {
               Guid componentGroupEditorId = componentGroupPair.Key;
               XmlDocument componentGroupDoc = componentGroupPair.Value;

               MemoryStream tempStream = new MemoryStream();
               // Move off doc root and component group to component as we only send component properties.
               XmlNodeReader componentNodeReader = new XmlNodeReader( Tools.Common.XmlNodeHelpers.GetFirstChildElement( Tools.Common.XmlNodeHelpers.GetDocumentRootElement( componentGroupDoc ) ) );
               componentNodeReader.Read(); // Move off the root
               
               CookProperties.CookInfo cookInfo = new CookProperties.CookInfo();
               CookProperties.CookOutput outputDependencies = new CookProperties.CookOutput();

               // Add custom cookers (normally splines)
               cookInfo.mCustomPropertyCooker = mCustomPropertyCookers;
               cookInfo.mComponentGroupPropertiesModifier = mComponentGroupPropertiesModifier;

               mComponentGroupPropertiesModifier( Tools.Common.XmlNodeHelpers.GetDocumentRootElement( componentGroupDoc ), assetManager, cookInfo );

               CookProperties.WriteCookedProperties( assetManager, componentNodeReader, cookInfo, outputDependencies, tempStream, componentGroupEditorId ); // This will also write message properties too, but those will be ignored gameside so we'll send them anyway
               if (tempStream.Length == 0) continue;  // Didn't cook correctly

               // Now read out hash, size, guid
               BinaryReader reader = new BinaryReader(new MemoryStream(tempStream.ToArray()));
               uint hash = reader.ReadUInt32(); // Ignore this hash, not used
               ushort size = reader.ReadUInt16();
               bool componentEditorIdPresent = reader.ReadBoolean();
               Guid componentGuid = new Guid(reader.ReadBytes(16));

               // Build full chunk of data before sending it down network connection as GameConnection.exe expects an entire data chunk to be available
               MemoryStream fullPacketStream = new MemoryStream();
               BinaryWriterBigEndian outputStream = new BinaryWriterBigEndian(fullPacketStream);

               long remainder = ( reader.BaseStream.Length - reader.BaseStream.Position );
               long dataSize = remainder + 1; // bool (false - we'll ignore the editor id)

               // Header
               outputStream.Write(Tools.Common.Misc.GetFourCCFromString("NPRP"));
               outputStream.Write((uint)dataSize + 16);  // Component editor id guid so receiver knows the destination component.
               outputStream.Write(componentGuid.ToByteArray());
               outputStream.Write((uint)dataSize);

               // Write data
               // We removed the component guids already so we'll just write a false bool to say that they aren't present
               bool bHasEditorIds = false;
               outputStream.Write(bHasEditorIds);
               outputStream.Write(reader.ReadBytes((int)remainder)); // Write remainder of stream
               outputStream.Flush();

               // Now send over socket
               WritetoConnectionWithRetry( fullPacketStream.GetBuffer(), 0, (int) fullPacketStream.Length );
               mConnection.GetStream().Flush();
            }

            mCachedSendData.Clear();
         }
      }
   }
}

