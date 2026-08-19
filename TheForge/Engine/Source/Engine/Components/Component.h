#pragma once
#include <json.hpp>

#include "ByteStream.h"
#include "..\Utilities/ReflectionMacros.h"

using json = nlohmann::json;

namespace Engine
{
    class GameObject;
    class Component
    {
        friend class NetCode::InputByteStream;
        friend class NetCode::OutputByteStream;
        
    public:

        Component() = default;
        virtual ~Component() = default;

        virtual void Deserialize(const json& data) {}
        virtual nlohmann::json Serialize() { return {}; }

        // Called before the first update frame for this object
        virtual void Start() {}
        
        // Called every frame from the game engine
        virtual void Update(float deltaTime) {}
        
        // Triggers when component is activated within the pool
        // May not have access to other component on the game object 
        virtual void OnActivation() {}

        // The component in full. Used for the complete world state, and for any object a
        // peer has not seen before -- enough to construct the thing from nothing.
        virtual void Read(NetCode::InputByteStream& stream) {}
        virtual void Write(NetCode::OutputByteStream& stream) const {}

        // What goes into a per-tick delta, for components with isReplicated set.
        //
        // Defaults to the members marked REPLICATE(), and falls back to the full Write
        // when the component marks none. That fallback is what makes adoption
        // incremental: setting isReplicated replicates the whole component immediately,
        // and adding REPLICATE() to its fields narrows it later without touching this.
        virtual void WriteDelta(NetCode::OutputByteStream& stream) const;
        virtual void ReadDelta(NetCode::InputByteStream& stream);

        // Null unless the class uses REFLECT(). REFLECT() declares an override with this
        // exact signature, so a Component* can ask any component what it knows about
        // itself without every component having to opt in.
        virtual ReflectionInfo* GetInstanceReflectionInfo() const { return nullptr; }

        virtual void DrawDetails() {}

    protected:
        // Walk this component's REPLICATE()-marked members. Return false when there is
        // no reflection info or nothing is marked, which is the caller's signal to fall
        // back to the hand-written Write/Read.
        bool WriteReplicatedMembers(NetCode::OutputByteStream& stream) const;
        bool ReadReplicatedMembers(NetCode::InputByteStream& stream);

    public:
        GameObject* gameObject = nullptr;

        // Whether this component appears in a per-tick delta at all.
        //
        // Deliberately false by default. A delta used to carry every component in full,
        // every tick -- a Player pawn cost ~1713 bytes because its renderers resent
        // their sprite and shader *file paths* 30 times a second. Only components that
        // actually carry changing state should opt in; anything set once at spawn
        // already travels in the full object record.
        bool isReplicated = false;
        
    };
}
