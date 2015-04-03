#ifndef LEVIATHAN_ENTITY_BRUSH
#define LEVIATHAN_ENTITY_BRUSH
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif

// ------------------------------------ //
// ---- includes ---- //
#include "Entities/Bases/BaseObject.h"
#include "Entities/Bases/BaseRenderable.h"
#include "Entities/Bases/BasePositionable.h"
#include "Entities/Bases/BasePhysicsObject.h"
#include "Entities/Bases/BaseConstraintable.h"
#include "Entities/Bases/BaseParentable.h"
#include "Entities/Bases/BaseSendableEntity.h"
#ifdef NETWORK_USE_SNAPSHOTS
#include "Entities/Bases/BaseTimedInterpolated.h"
#include "Events/CallableObject.h"
#else
#include "Entities/Bases/BaseInterpolated.h"
#endif //NETWORK_USE_SNAPSHOTS

namespace Leviathan{
	class GameWorld;
}

namespace Leviathan{ namespace Entity{

        //! \brief A (potentially) movable brush
        //! \todo Make sure that _MarkDataUpdated is called enough
        class Brush : public virtual BaseObject, public virtual BaseRenderable, public BaseConstraintable,
                        public BaseParentable, public BaseSendableEntity, public BasePhysicsObject,
#ifdef NETWORK_USE_SNAPSHOTS
                        public BaseTimedInterpolated, public virtual CallableObject
#else
                        public BaseInterpolated
#endif //NETWORK_USE_SNAPSHOTS
        {
            friend BaseSendableEntity;
        public:
            DLLEXPORT Brush(bool hidden, GameWorld* world);
            DLLEXPORT virtual ~Brush();

            DLLEXPORT virtual void ReleaseData();

            // different initialization functions for different box styles //
            // NOTE: leaving createphysics true creates a immovable box (uses mass = 0) //
            DLLEXPORT bool Init(const Float3 &dimensions, const string &material, bool createphysics = true);

            // call if you want to have this interact with other physical objects (set mass to 0 to be static) //
            DLLEXPORT void AddPhysicalObject(const float &mass = 0.f);


            DLLEXPORT virtual bool SendCustomMessage(int entitycustommessagetype, void* dataptr);

            static void BrushPhysicsMovedEvent(const NewtonBody* const body, const dFloat* const
                matrix, int threadIndex);

            //! \copydoc BaseSendableEntity::CaptureState
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CaptureState(int tick) override;

#ifndef NETWORK_USE_SNAPSHOTS
            
            //! \copydoc BaseSendableEntity::VerifyOldState
            DLLEXPORT virtual void VerifyOldState(ObjectDeltaStateData* serversold,
                ObjectDeltaStateData* ourold, int tick) override;

#else

            DLLEXPORT int OnEvent(Event** pEvent) override;
            DLLEXPORT int OnGenericEvent(GenericEvent** pevent) override;

            DLLEXPORT bool SetStateToInterpolated(ObjectDeltaStateData &first, ObjectDeltaStateData &second,
                float progress);

#endif //NETWORK_USE_SNAPSHOTS

            //! \copydoc BaseSendableEntity::CreateStateFromPacket
            DLLEXPORT virtual shared_ptr<ObjectDeltaStateData> CreateStateFromPacket(int tick,
                sf::Packet &packet) const override;

            REFERENCECOUNTED_ADD_PROXIESFORANGELSCRIPT_DEFINITIONS(Brush);
            
        protected:

            //! \brief Constructs a brush for receiving through the network
            Brush(bool hidden, GameWorld* world, int netid);
            
            virtual void _UpdatePhysicsObjectLocation(ObjectLock &guard) override;

            //! \copydoc BaseSendableEntity::_LoadOwnDataFromPacket
            virtual bool _LoadOwnDataFromPacket(sf::Packet &packet) override;

            //! \copydoc BaseSendableEntity::_SaveOwnDataToPacket
            virtual void _SaveOwnDataToPacket(sf::Packet &packet) override;
            
            //! \copydoc BaseConstraintable::_SendCreatedConstraint
            void _SendCreatedConstraint(BaseConstraintable* other, Entity::BaseConstraint* ptr) override;

            //! \copydoc BasePhysicsObject::OnBeforeResimulateStateChanged
            virtual void OnBeforeResimulateStateChanged() override;

#ifdef NETWORK_USE_SNAPSHOTS
            
            void VerifySendableInterpolation() override;

            bool OnInterpolationFinished() override;

#else
            //! \copydoc BaseInterpolated::_GetCurrentActualPosition
            void _GetCurrentActualPosition(Float3 &pos) override;
        
            //! \copydoc BaseInterpolated::_GetCurrentActualRotation
            void _GetCurrentActualRotation(Float4 &rot) override;

#endif //NETWORK_USE_SNAPSHOTS
            
            BaseConstraintable* BasePhysicsGetConstraintable() override;            
            // ------------------------------------ //

            Ogre::ManualObject* BrushModel;

            string MeshName;
            string Material;
            float Mass;
            Float3 Sizes;
        };

    }}

#endif
