#pragma once
#include <typeindex>
#include <vector>

#include "json.hpp"
#include "ByteStream.h"
#include "LinkingContext.h"
#include "NetworkManager.h"
#include "Utilities/Array.h"

namespace Engine
{
    class Component;
    class GameObject
    {
        friend class Level;
        friend class NetCode::NetworkManager;
        friend class Transform;
        
    public:
        GameObject();
        GameObject(const nlohmann::json& data);
        ~GameObject();

        // Component Logic
        template <class T>
        T* GetComponent() const;
        
        std::vector<Component*> GetAllComponents() const;

        template <typename T>
        T* AddComponent();

        void RemoveComponent(Component* component);
        
        // Parent-Children
        void AddChild(GameObject* child, bool keepWorldPosition = false);
        void RemoveChild(GameObject* child);
        
        // Serialization
        void Deserialize(const nlohmann::json& data);
        nlohmann::json Serialize();
    
    private:
        void UpdateWorldTransform() const;
        
        // The whole object: identity, hierarchy and every component in full. Enough to
        // construct it from nothing, so it is what a peer that has never seen this
        // object gets -- in the complete world state, or in a delta the first time.
        void Write(NetCode::OutputByteStream& stream);
        void Read(NetCode::InputByteStream& stream);

        // The per-tick form: only components with isReplicated, and only their changing
        // members. Name, parent and child links are omitted entirely -- they are
        // established by the full form and a peer only ever gets a delta for an object
        // it already has.
        void WriteDelta(NetCode::OutputByteStream& stream) const;
        void ReadDelta(NetCode::InputByteStream& stream);

        void LinkFamily();
        
    public:
        bool isReplicated = true;
        bool isServerOnly = false;

        // Flags this object as needing to go out to every peer the level replicates
        // to. Dirtiness is tracked per peer rather than as one shared flag: a world
        // state delta *clears* what it sends, so a single flag meant an update that
        // one backed-up peer could not accept was cleared for everyone and that peer
        // silently desynced -- and it made skipping a peer impossible for the same
        // reason. A peer's bit is retired only once its own delta has actually gone.
        //
        // Every bit is set here rather than only the currently-active ones, so this
        // needs to know nothing about the level or the session. Level masks the
        // result against the slots that are really in use.
        void MarkDirty() { _dirtyPeers = ALL_PEERS_DIRTY; }
        
    private:
        static constexpr uint32_t ALL_PEERS_DIRTY = 0xFFFFFFFFu;

        // Bit N is set while the peer in the level's replication slot N is still
        // owed this object's state. Width is NetCode::MAX_REPLICATION_PEERS.
        uint32_t _dirtyPeers = 0;

        // Bit N is set once the peer in slot N has been sent this object in full. It is
        // what decides which of the two forms above that peer gets: a delta describes a
        // change to something the receiver already has, so sending one for an object it
        // has never seen is unreadable -- there is no length prefix to skip past.
        uint32_t _knownToPeers = 0;

        GameObject* _parent;
        Array<GameObject*> _children;
        std::string _name;
        std::unordered_map<std::type_index, Component*> _components;

        // Only contains the default data used when the game object is first created
        // TODO: Can be moved to a utils file
        nlohmann::json _defaultData;

        uint32_t _parentNID = NULL_ID;
        Array<uint32_t> _childrenNIDs;

    public:
        void SetName(const std::string& name) { _name = name; }
        std::string GetName() const { return _name; }

        // Sets the *local* position. Physics and collision resolution want the world
        // one -- see SetWorldPosition.
        void SetPosition(const glm::vec2& position) const;

        // Places the object at a world position, expressed in whatever frame its parent
        // establishes. A Transform stores a local position, so writing a world position
        // straight into it is correct only for an object that has no parent -- on a
        // child it displaces the object by the parent's offset and rotation, every time.
        void SetWorldPosition(const glm::vec2& position) const;

        glm::vec2 GetWorldPosition() const;
        glm::vec2 GetLocalPosition() const;

        void SetRotation(float rotation) const;
        float GetWorldRotation() const;

        Array<GameObject*> GetChildren() const { return _children; }
        GameObject* GetParent() const { return _parent; }
        
    };
}

#include "GameObject.inl"
