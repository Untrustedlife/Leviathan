// Leviathan Game Engine
// Copyright (c) 2012-2017 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include <type_traits>

#include "Networking/CommonNetwork.h"

#include "Systems.h"
#include "Components.h"

#include "Newton/PhysicalWorld.h"
#include "Common/ReferenceCounted.h"


#define PHYSICS_BASE_GRAVITY		-9.81f

namespace Ogre{

class CompositorWorkspace;
}

namespace Leviathan{


#define WORLD_CLOCK_SYNC_PACKETS 12
#define WORLD_CLOCK_SYNC_ALLOW_FAILS 2
#define WORLD_OBJECT_UPDATE_CLIENTS_INTERVAL 2
    
//! Holds the returned object that was hit during ray casting
//! \todo Move to a new file
class RayCastHitEntity : public ReferenceCounted{
public:
    DLLEXPORT RayCastHitEntity(const NewtonBody* ptr = NULL, const float &tvar = 0.f,
        RayCastData* ownerptr = NULL);

    DLLEXPORT RayCastHitEntity& operator =(const RayCastHitEntity& other);

    // Compares the hit entity with NULL //
    DLLEXPORT bool HasHit();

    DLLEXPORT Float3 GetPosition();

    DLLEXPORT bool DoesBodyMatchThisHit(NewtonBody* other);

    //! Stores the entity, typed as NewtonBody to make sure that user knows
    //! what should be compared with this
    const NewtonBody* HitEntity;

    //! The distance from the start of the ray to the hit location
    float HitVariable;
    Float3 HitLocation;
};

// Internal object in ray casts //
struct RayCastData{
    DLLEXPORT RayCastData(int maxcount, const Float3 &from, const Float3 &to);
    DLLEXPORT ~RayCastData();

    // All hit entities that pass checks //
    std::vector<RayCastHitEntity*> HitEntities;
    // Used to stop after certain amount of entities found //
    int MaxCount;
    // Used to efficiently calculate many hit locations //
    Float3 BaseHitLocationCalcVar;
};

//! \brief Represents a world that contains entities
//!
//! This is the base class from which worlds that support different components are derived from
//! Custom worls should derive from StandardWorld which has all of the standard components
//! supported. See the GenerateStandardWorld.rb file to figure out how to generate world
//! classes
class GameWorld{
public:
    DLLEXPORT GameWorld();
    DLLEXPORT ~GameWorld();

    //! \brief Returns the unique ID of this world
    DLLEXPORT inline int GetID() const{
        return ID;
    }

    //! \brief Creates resources for the world to work
    //! \post The world can be used after this
    DLLEXPORT bool Init(NETWORKED_TYPE type, GraphicalInputEntity* renderto, Ogre::Root* ogre);

    //! Release to not use Ogre when deleting
    DLLEXPORT void Release();

    //! \brief Marks all objects to be deleted
    DLLEXPORT void MarkForClear();

    //! Clears all objects from the world
    DLLEXPORT void ClearObjects();

    //! \brief Returns the number of ObjectIDs this world keeps track of
    //! \note There may actually be more objects as it is possible to create components
    //! for ids that are not created (this is not recommended but it isn't enforced)
    DLLEXPORT size_t GetObjectCount() const;


    //! \brief Used to keep track of passed ticks and trigger timed triggers
    //! \note This will be called (or should be) every time the engine ticks
    //! \note This cannot be used for accurate time keeping for that use timers, but for
    //! events that need to happen at certain game world times this is ideal
    DLLEXPORT void Tick(int currenttick);

    //! \brief Runs systems required for a rendering run. Also updates camera positions
    DLLEXPORT void Render(int mspassed, int tick, int timeintick);

    //! \brief Returns the current tick
    DLLEXPORT int GetTickNumber() const;

    //! \brief Returns float between 0.f and 1.f based on how far current tick has progressed
    DLLEXPORT float GetTickProgress() const;
    
    
    //! \brief Fetches the physical material ID from the material manager
    DLLEXPORT int GetPhysicalMaterial(const std::string &name);

    //! \todo Expose the parameters and make this activate the fog
    DLLEXPORT void SetFog();
    DLLEXPORT void SetSkyBox(const std::string &materialname);

    //! \brief Alternative to skybox This is a (possibly curved) plane
    //! attached to the camera that can be used to render a background or
    //! a sky
    DLLEXPORT void SetSkyPlane(const std::string &material, const Ogre::Plane &plane =
        Ogre::Plane(1, 1, 1, 1));

    //! \brief Disables sky plane
    //! \pre SetSkyPlane has been used to set a sky plane
    //! \post The sky plane is disabled
    DLLEXPORT void DisableSkyPlane();
    

    DLLEXPORT void SetSunlight();
    DLLEXPORT void RemoveSunlight();


    //! \brief Casts a ray from point along a vector and returns the first physical
    //! object it hits
    //! \warning You need to call Release on the returned object once done
    DLLEXPORT RayCastHitEntity* CastRayGetFirstHit(const Float3 &from, const Float3 &to);

    //! \brief Creates a new empty entity and returns its id
    DLLEXPORT ObjectID CreateEntity();

    //! \brief Destroys an entity and all of its components
    //! \todo Make this less expensive
    DLLEXPORT void DestroyObject(ObjectID id);

    //! \brief Deletes an entity during the next tick
    DLLEXPORT void QueueDestroyObject(ObjectID id);

    //! \brief Notifies others that we have created a new entity
    //! \note This is called after all components are set up and it is ready to be sent to
    //! other players
    //! \note Clients should also call this function
    //! \todo Allow to set the world to queue objects and send them in
    //!big bunches to players
    DLLEXPORT void NotifyEntityCreate(ObjectID id);


    //! \brief Removes all components from an entity
    DLLEXPORT virtual void DestroyAllIn(ObjectID id);

    //! Helper for getting component of type. This is much slower than
    //! direct lookups with the actual implementation class' GetComponent_Position etc.
    //! methods
    //! \exception NotFound if entity has no component of the wanted type
    template<class TComponent>
        TComponent& GetComponent(ObjectID id){

        std::tuple<void*, bool> component = GetComponent(id, TComponent::TYPE);

        if(!std::get<1>(component))
            throw InvalidArgument("Unrecognized component type as template parameter");

        void* ptr = std::get<0>(component);

        if(!ptr)
            throw NotFound("Component for entity with id was not found");
        
        return *static_cast<TComponent*>(ptr);
    }

    //! \brief Gets a component of type or returns nullptr
    //!
    //! \returns Tuple of pointer to component and boolean indicating if the type is known
    DLLEXPORT virtual std::tuple<void*, bool> GetComponent(ObjectID id, COMPONENT_TYPE type);


    //! \brief Sets the entity that acts as a camera.
    //!
    //! The entity needs atleast Position and Camera components
    //! \exception InvalidArgument if the object is missing required components
    DLLEXPORT void SetCamera(ObjectID object);
    
    // Ogre get functions //
    inline Ogre::SceneManager* GetScene(){
        return WorldsScene;
    }
    
    // physics functions //
    DLLEXPORT Float3 GetGravityAtPosition(const Float3 &pos);

    inline PhysicalWorld* GetPhysicalWorld(){
        return _PhysicalWorld.get();
    }

    //! \todo Synchronize this over the network
    DLLEXPORT void SetWorldPhysicsFrozenState(bool frozen);

    // Ray callbacks //
    static dFloat RayCallbackDataCallbackClosest(const NewtonBody* const body,
        const NewtonCollision* const shapeHit, const dFloat* const hitContact,
        const dFloat* const hitNormal, dLong collisionID, void* const userData,
        dFloat intersectParam);
		
    // Script proxies //
    DLLEXPORT RayCastHitEntity* CastRayGetFirstHitProxy(const Float3 &from, const Float3 &to);

    //! \brief Returns true when the player matching the connection should receive updates
    //! about an object
    //! \todo Implement this
    DLLEXPORT bool ShouldPlayerReceiveObject(Position &atposition,
        Connection &connection);

    //! \brief Returns true if a player with the given connection is receiving updates for
    //! this world
    DLLEXPORT bool IsConnectionInWorld(Connection &connection) const;

    //! \brief Verifies that player is receiving this world
    DLLEXPORT void SetPlayerReceiveWorld(std::shared_ptr<ConnectedPlayer> ply);

    //! \brief Sends a packet to all connected players
    DLLEXPORT void SendToAllPlayers(const std::shared_ptr<NetworkResponse> &response,
        RECEIVE_GUARANTEE guarantee) const;

    //! \brief Sends an object to a connection and sets everything up
    //! \post The connection will receive updates from the object
    //! \return True when a packet was sent false otherwise
    DLLEXPORT bool SendObjectToConnection(ObjectID obj,
        std::shared_ptr<Connection> connection);
        
    //! \brief Creates a new entity from initial entity response
    //! \note This should only be called on the client
    DLLEXPORT void HandleEntityInitialPacket(std::shared_ptr<NetworkResponse> message,
        ResponseEntityCreation* data);

    //! \brief Applies an update packet
    //!
    //! If the entity is not found the packet is discarded
    //! \todo Cache the update data for 1 second and apply it if a matching entity is
    //! created during that time
    DLLEXPORT void HandleEntityUpdatePacket(std::shared_ptr<NetworkResponse> message);

    //! \brief Handles a world clock synchronizing packet
    //! \note This should only be allowed to be called on a client that has connected
    //! to a server
    DLLEXPORT void HandleClockSyncPacket(RequestWorldClockSync* data);

    //! \brief Handles a world freeze/unfreeze packet
    //! \note Should only be called on a client
    DLLEXPORT void HandleWorldFrozenPacket(ResponseWorldFrozen* data);

    //! \brief Applies packets that have been received after the last call to this
    DLLEXPORT void ApplyQueuedPackets();

    //! \brief Called when a component is destroyed, used to destroy nodes
    DLLEXPORT void _OnComponentDestroyed(ObjectID id, COMPONENT_TYPE type);

    //! \brief Use this to register destruction events for child classes
    DLLEXPORT virtual void _OnCustomComponentDestroyed(ObjectID id, COMPONENT_TYPE type);
    
protected:

    //! \brief Called by Render which is called from a
    //! GraphicalInputEntity if this is linked to one
    DLLEXPORT virtual void RunFrameRenderSystems(int tick, int timeintick);
    

    //! \brief Handles added entities and components
    //!
    //! Construct new nodes based on components values. This is split
    //! into multiple parts to support child classes using the same
    //! Component types in additional nodes
    DLLEXPORT virtual void HandleAdded();

    //! \brief Clears the added components. Call after HandleAdded
    DLLEXPORT virtual void ClearAdded();
    
    //! \brief Resets stored nodes in systems. Used together with _ResetComponents
    DLLEXPORT virtual void _ResetSystems() = 0;

    //! \brief Resets components in holders. Used together with _ResetSystems
    DLLEXPORT virtual void _ResetComponents() = 0;
    
private:

    //! \brief Updates a players position info in this world
    void UpdatePlayersPositionData(ConnectedPlayer &ply);

    void _CreateOgreResources(Ogre::Root* ogre, GraphicalInputEntity* rendertarget);
    void _HandleDelayedDelete();

    //! \brief Reports an entity deletion to clients
    //! \todo Potentially send these in a big blob
    void _ReportEntityDestruction(ObjectID id);

    //! \brief Implementation of doing actual destroy part of removing an entity
    void _DoDestroy(ObjectID id);

    //! \brief Sends sendable updates to all clients
    void _SendEntityUpdates(ObjectID id, Sendable &sendable, int tick);


    // Packet apply functions //
    void _ApplyInitialEntityPackets();

    void _ApplyEntityUpdatePackets();

protected:

    //! \brief If false a graphical Ogre window hasn't been created
    //! and purely graphical stuff should be skipped
    //!
    //! Used on dedicated servers and other headless applications
    bool GraphicalMode = false;

    //! Bool flag telling whether this is a master world (on a server) or
    //! a mirroring world (client)
    bool IsOnServer = false;
    
private:

    Ogre::Camera* WorldSceneCamera = nullptr;
    Ogre::SceneManager* WorldsScene = nullptr;

    Ogre::CompositorWorkspace* WorldWorkspace = nullptr;

    //! The world is now always linked to a window
    GraphicalInputEntity* LinkedToWindow = nullptr;

    Ogre::Light* Sunlight = nullptr;
    Ogre::SceneNode* SunLightNode = nullptr;

    // physics //
    std::shared_ptr<PhysicalWorld> _PhysicalWorld;

    //! The world can be frozen to stop physics
    bool WorldFrozen = false;


    //! Marks all objects to be released
    bool ClearAllObjects = false;

    //! Holds the players who are receiving this worlds updates and their corresponding
    //! location entities (if any)
    //! \todo Change this to an object that holds more than the player pointer
    std::vector<std::shared_ptr<ConnectedPlayer>> ReceivingPlayers;

    // objects //
    std::vector<ObjectID> Objects;

    //! The unique ID
    int ID;

    //! The current tick number
    //! This should be the same on all clients as closely as possible
    int TickNumber = 0;

    //! A funky name for this world, if any
    std::string DecoratedName;

    //! If not zero controls the position and properties of WorldSceneCamera
    ObjectID CameraEntity = 0;
    
    //! The currently applied properties on WorldSceneCamera if the
    //! Camera component of CameraEntity changes (or it is Marked)
    //! these properties are set on WorldSceneCamera
    Camera* AppliedCameraPropertiesPtr = nullptr;
    

    //! A lock for delayed delete, to allow deleting objects from physical callbacks
    Mutex DeleteMutex;
        
    //! This vector is used for delayed deletion
    std::vector<ObjectID> DelayedDeleteIDS;

    //! If true any pointers to this world are invalid
    std::shared_ptr<bool> WorldDestroyed = std::make_shared<bool>(false);

    //! Mutex for ConstraintList
    Mutex ConstraintListMutex;

    //! List of constraints in this world
    //!
    //! Used to send full lists to clients
    std::vector<std::shared_ptr<BaseConstraint>> ConstraintList;

    //! Waiting entity packets
    std::vector<std::shared_ptr<NetworkResponse>> InitialEntityPackets;

    //! Waiting update packets
    std::vector<std::shared_ptr<NetworkResponse>> EntityUpdatePackets;
};
    
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::GameWorld;
using Leviathan::ObjectID;
#endif

