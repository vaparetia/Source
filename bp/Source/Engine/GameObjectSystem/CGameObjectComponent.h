//------------------------------------------------------------------------------------------
// CGameObjectComponent.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"

#include "TGameObjectTypes.h"
#include "CGameObjectMessage.h"
#include "Engine/Evaluators/CEvaluatorUpdateData.h"
//------------------------------------------------------------------------------------------

class CGameObject;
class CBaseGameManager;
class CGameObjectConstructionParams;
class CGameObjectComponentProperties;
class CGameObjectComponentMethods;
class TiXmlNode;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectComponent
{
public:
   typedef std::vector<CGameObjectMessage>      TMessages;

   explicit CGameObjectComponent( boost::shared_ptr<CGameObjectComponentProperties const > pProperties );
   virtual ~CGameObjectComponent();

   TComponentUniqueId const &    GetId() const { return mId; }
   TComponentEditorId const &    GetEditorId() const;
   TGameObjectUniqueId const &   GetGameObjectId() const;
   TGameObjectEditorId const &   GetGameObjectEditorId() const;

   CGameObject const *  GetGameObject() const { return mpGameObjectOwner; }     // Gets the game object that owns this component
   CGameObject *        GameObject() { return mpGameObjectOwner; }

   virtual void         Initialize(CBaseGameManager &baseGameManager, CGameObject &gameObjectOwner, TComponentUniqueId const &componentUniqueId, CGameObjectConstructionParams const &params);
   /// Called when all objects within it's package have been initialized. Used for initialization that requires other objects.
   virtual void         PostLoadInitialize(CBaseGameManager &baseGameManager);
   virtual void         Uninitialize(CBaseGameManager &baseGameManager);
   virtual void         BindMethods(CGameObjectComponentMethods &methods) const;
   virtual void         PropertiesChanged(); // Called when component properties have been modified by an external source
   virtual void         InternalProcessMessage(CBaseGameManager &baseGameManager, CGameObjectMessage const &message, CGameObjectMessageInfo const &info);
   virtual bool         RunQuery(std::string const &queryString) const;
   bool                 HasEvent(int32 const eventId) const;   // Returns false if event not present, useful for optimization

   // Most commonly used function for triggering events.
   void                 SendEventMessages(CBaseGameManager &manager,
                                          int32 const eventId, 
                                          TComponentUniqueId const * const pOriginator = NULL, 
                                          std::string const * const pAdditionalDestQuery = NULL) const;

   // Helper function for sending discrete messages.
   void                 SendSingleMessage(CBaseGameManager &manager, 
                                          CGameObjectMessage const &message, 
                                          TComponentUniqueId const * const pOriginator = NULL, 
                                          std::string const * const pAdditionalDestQuery = NULL) const;

   // Helper function for sending a custom message.
   void                 SendSimpleMessage(CBaseGameManager &manager, 
                                          int32 const message, 
                                          TComponentUniqueId const &destId, 
                                          TComponentUniqueId const * const pOriginatorId = NULL, 
                                          real32 const time = 0.0f, 
                                          int32 const priority = 0, 
                                          std::string const &senderQuery = std::string(),
                                          std::string const &destQuery = std::string(),
                                          std::string const &additionalDestQuery = std::string()) const;

   // Helper function for deleting object safely.
   void                 DeleteYourself(CBaseGameManager &manager);

   boost::shared_ptr<CGameObjectComponentProperties const > const &
                        GetProperties() const   { return mpProperties; };

   virtual void         DuplicateProperties();
   bool                 HasDuplicatedProperties() const;

   void                 DuplicateMessagesAndLinks();
   void                 RemapMessagesAndLinksToUniqueIds(CBaseGameManager &manager, CGameObject const &remapParent, CGameObject const * const pRemapToGenerator, int32 const * const pExcludeMessageType = NULL);

   // Helper functions for getting update data.
   // You can write your own or using something completely different if you need to.
   CEvaluatorUpdateData          GetInitializationEvaluatorUpdateData(CBaseGameManager &manager);
   CEvaluatorUpdateData          GetEvaluatorUpdateData(CBaseGameManager &manager);
   // Use this to get update data for message objects, manages any allocated memory
   // Use the allocator associated with the message object.
   CManagedEvaluatorUpdateData   GetTransientEvaluatorUpdateData(CBaseGameManager &manager, CEvaluatorAllocator const &allocator) const; 

   // Returns the number of linked components
   int                           GetLinkedComponents(CBaseGameManager const &manager, int32 const linkId, TConstComponentListTransient &linkedComponents) const; // Results are transient, don't hang onto them!
   int                           LinkedComponents(CBaseGameManager &manager, int32 const linkId, TComponentListTransient &linkedComponents) const; // Results are transient, don't hang onto them!
   TComponentId const &          GetFirstLinkedComponentId(int32 const linkId) const;
   CGameObjectComponent *        FirstLinkedComponent(int32 const linkId, CBaseGameManager &manager, std::string const &destQuery = std::string());
   CGameObjectComponent const *  GetFirstLinkedComponent(int32 const linkId, CBaseGameManager const &manager, std::string const &destQuery = std::string()) const;
   CGameObjectComponent *        FirstLinkedUniqueIdComponent(int32 const linkId, CBaseGameManager &manager, std::string const &destQuery = std::string());     //  Returns first component linked using a unique id
   CGameObjectComponent const *  GetFirstLinkedUniqueIdComponent(int32 const linkId, CBaseGameManager const &manager, std::string const &destQuery = std::string()) const;

   TMessages const &             GetRemappedMessages() const      { return mMessages; };
   TMessages &                   RemappedMessages()               { return mMessages; };
   TMessages const &             GetRemappedLinks() const         { return mLinks; };
   TMessages &                   RemappedLinks()                  { return mLinks; };
   // Returns remapped if present otherwise returns ones stored in properties
   TMessages const &             GetMessages() const;
   TMessages const &             GetLinks() const;

   // Helper function that retrieves editor name from CGameObject
   std::string const &              GetEditorName() const;

   // Debug logging
   virtual TiXmlNode *  LogInfo(CBaseGameManager const &manager, TiXmlNode * pXml) const;
   // Used by debug logging to convert message/event types to strings
   virtual char const * GetMessageTypeName(int32 const messageType) const;
   virtual char const * GetEventTypeName(int32 const eventType) const;

protected:
   void                          AllocateEvaluatorDataBlock();
   boost::scoped_ptr<uint8>                     mpEvaluatorDataBlock;

private:
   CGameObject *                                mpGameObjectOwner;
   TComponentUniqueId                           mId;
   boost::shared_ptr<CGameObjectComponentProperties const > 
                                                mpProperties;
   boost::shared_ptr<CGameObjectComponentProperties const > 
                                                mpOriginalProperties;
   TMessages                                    mMessages;     // Used to override mpProperties->mMessage if size is non zero
   TMessages                                    mLinks;        // Similar to above except for links

};

//------------------------------------------------------------------------------------------
