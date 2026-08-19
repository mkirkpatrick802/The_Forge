#pragma once
#include <string>
#include <vector>

#include "ByteStream.h"
#include "GameEngine.h"
#include "INetTransport.h"
#include "json.hpp"
#include "LevelManager.h"

namespace NetCode
{
    class NetworkManager;
}

namespace Editor
{
    class LevelEditor;
    class DetailsEditor;
}

namespace Engine
{
    // How far from a peer's own pawn an object is still replicated to it.
    //
    // Must comfortably exceed what a client can actually see or objects pop in at the
    // screen edge. The camera's world extent is windowSize / zoom, so a 1920x1080 client
    // at zoom 1 sees about 1100 units to the corner; this leaves room to zoom out to
    // roughly a third of that before the horizon becomes visible. The test level is
    // 10000 units square, so this genuinely excludes most of it.
    constexpr float RELEVANCE_ENTER_RADIUS = 3000.0f;

    // Hysteresis, deliberately larger than the enter radius. An object drifting on the
    // boundary would otherwise be dropped and re-sent every few ticks -- and each
    // re-send is a *full* record, which is precisely the traffic this exists to avoid.
    constexpr float RELEVANCE_LEAVE_RADIUS = 3600.0f;

    class GameModeBase;
    class Level;
    class GameObject;
    
    class Level
    {
        friend class LevelManager;
        friend class Editor::DetailsEditor;
        friend class NetCode::NetworkManager;
        friend class Editor::LevelEditor;
        
    public:
        GameObject* SpawnNewGameObject(const std::string& filepath = "", glm::vec2 position = glm::vec2(0.0f, 0.0f));
        GameObject* SpawnNewGameObjectFromJson(const nlohmann::json& data);
        GameObject* SpawnNewGameObjectFromInputStream(NetCode::InputByteStream& stream, uint32_t NID);
        bool RemoveGameObject(GameObject* go, bool replicate = true);

        void SaveLevel(const std::string& args = "");
        
        std::vector<GameObject*> GetAllGameObjects() const;
        std::string GetName() { return _name; }
        GameModeBase& GetGameMode() const { return *_gameMode; }
        
    private:
        // The world as one peer should first see it: everything within its relevance
        // radius, plus everything exempt from relevance, each in full form. Marks what it
        // wrote as known to that peer.
        void WriteCompleteStateFor(NetCode::PeerID peer, NetCode::OutputByteStream& stream);
        void Read(NetCode::InputByteStream& stream);

        // --- per-peer replication ---
        //
        // A world state delta clears the dirty state it sends, so it can only ever be
        // sent once. While that state was global this made a broadcast unsafe in both
        // directions: an update refused by one peer's full send window was cleared for
        // everyone and that peer desynced with nothing to detect it, and a backed-up
        // peer could not simply be skipped for the same reason. Tracking dirtiness per
        // peer is what makes a send refusal survivable -- the peer keeps its bits, and
        // the update coalesces into its next one.
        //
        // Peers get a slot rather than being looked up by id because a GameObject's
        // dirty state is a bitmask: "does peer P still need this object" has to be one
        // bit test, not a map lookup, when it runs across every object for every peer
        // every tick.

        // Opens a slot for a peer, knowing nothing and owed nothing. The pawn is that
        // peer's viewpoint for relevance; a slot without one is sent everything, which is
        // the safe answer while a player has no position in the world yet.
        void AddReplicationPeer(NetCode::PeerID peer, GameObject* pawn);
        void RemoveReplicationPeer(NetCode::PeerID peer);

        // Serializes every object still owed to anybody, exactly once. Each peer's
        // update is then spliced out of these blobs, so the cost of a tick is one pass
        // over the dirty world rather than one pass per peer -- at 32 players that is
        // the difference between serializing the world once and 32 times.
        // Moves objects in and out of each peer's relevance set: an object entering is
        // marked dirty so it goes out in full, and one leaving is queued as a destruction
        // for that peer alone. Called at the top of BuildDeltaBlobs.
        //
        // O(peers x objects) per tick, which is fine at 32 peers and a few dozen objects
        // and is the first thing to want a spatial partition if either grows.
        void UpdateRelevance();

        // Relevance is asymmetric on purpose -- see RELEVANCE_LEAVE_RADIUS.
        static bool IsRelevantTo(const GameObject& object, const glm::vec2& viewpoint, bool currentlyKnown);

        void BuildDeltaBlobs();

        bool HasPendingDelta(NetCode::PeerID peer) const;

        // Appends the update `peer` is owed. Deliberately does not mutate anything:
        // what was written is only retired once the transport has accepted it.
        void WriteDeltaFor(NetCode::PeerID peer, NetCode::OutputByteStream& stream) const;

        // Retires what the matching WriteDeltaFor produced. Call only on a successful
        // send -- skipping it is what lets a refused update be re-sent instead of lost.
        void CommitDeltaFor(NetCode::PeerID peer);

        int FindReplicationSlot(NetCode::PeerID peer) const;
        
        Level();
        void Load(nlohmann::json data);
        void Load(NetCode::InputByteStream& stream);
        ~Level();

        void Start() const;
        
    private:
        std::string _name;
        std::string _path;
        glm::vec2 _size = glm::vec2(0.0f);
        
        std::vector<std::unique_ptr<GameObject>> _gameObjects;
        std::unique_ptr<GameModeBase> _gameMode; // Only exists on the server

        // One peer we replicate to. Destroyed objects are queued per peer for the same
        // reason dirty flags are tracked per peer: the old single list was drained by
        // whichever send happened first, so every other peer kept rendering an object
        // that no longer existed.
        struct ReplicationPeer
        {
            NetCode::PeerID peer = NetCode::INVALID_PEER;
            bool active = false;

            // This peer's own pawn, and so where it is looking from. Null until one
            // exists, and cleared by RemoveGameObject if the pawn is destroyed -- a
            // dangling viewpoint would be read every tick.
            GameObject* pawn = nullptr;

            // Network ids this peer specifically must drop: objects destroyed outright,
            // and objects that have simply left its relevance radius. The two are
            // indistinguishable to the receiver, and should be -- both mean "you no
            // longer have this".
            std::vector<uint32_t> pendingDestroys;
        };

        // One object serialized once for the tick, plus who still needs it.
        //
        // Two payloads because peers can be owed different *forms* of the same object in
        // the same tick: whoever has seen it before gets the small delta, and whoever
        // has not gets the full record. Each is built only if somebody actually needs
        // it, so the common steady state builds one.
        struct DeltaBlob
        {
            GameObject* object = nullptr;
            uint32_t networkID = 0;
            uint32_t owedFull = 0;  // slots that have never seen this object
            uint32_t owedDelta = 0; // slots that have, and just need the change
            NetCode::OutputByteStream fullPayload;
            NetCode::OutputByteStream deltaPayload;
        };

        ReplicationPeer _replicationPeers[NetCode::MAX_REPLICATION_PEERS];
        uint32_t _activePeerMask = 0;
        std::vector<DeltaBlob> _deltaBlobs;

    public:
        glm::vec2 GetLevelSize() const { return _size; }
        glm::vec2 GetLevelBottomLeft() const { return _size * -.5f; }
        glm::vec2 GetLevelTopLeft() const { return {_size.x * -.5f, _size.y * .5f}; }
    };

    inline void Destroy(GameObject* go)
    {
        LevelManager::GetCurrentLevel()->RemoveGameObject(go);
    }
}
