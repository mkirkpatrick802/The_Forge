#include "Level.h"
#include <fstream>

#include "CommandRegistry.h"
#include "GameModeBase.h"
#include "Components/Component.h"
#include "GameObject.h"
#include "JsonKeywords.h"
#include "LinkingContext.h"
#include "System.h"
#include "Components/Transform.h"

Engine::Level::Level()
{
    CommandRegistry::RegisterCommand("/save", [this](const std::string& args){ SaveLevel(args); });
    CommandRegistry::RegisterCommand("/gocount", [this](const std::string& args)
    {
        DEBUG_LOG("%d Game Objects", _gameObjects.size())
    });
}

Engine::Level::~Level()
{
    CommandRegistry::UnregisterCommand("/save");
    CommandRegistry::UnregisterCommand("/gocount");
    
    _gameObjects.clear();
}

void Engine::Level::Load(nlohmann::json data)
{
    _name = data[JsonKeywords::LEVEL_NAME];
    if (data.contains("Size X") && data.contains("Size Y"))
    {
        _size.x = data["Size X"];
        _size.y = data["Size Y"];
    }

    //Load Game Objects From Json Data
    if (!data.contains(JsonKeywords::GAMEOBJECT_ARRAY)) return;

    for (const auto& go_data : data[JsonKeywords::GAMEOBJECT_ARRAY])
    {
        auto go = std::make_unique<GameObject>(go_data);
        _gameObjects.push_back(std::move(go));
    }

    _gameMode = std::make_unique<GameModeBase>();
}

namespace
{
    // An object with no Transform has no position to test, so it is exempt from
    // relevance entirely rather than being treated as sitting at the origin --
    // GameObject::GetWorldPosition cannot tell those two apart.
    bool TryGetWorldPosition(const Engine::GameObject& object, glm::vec2& outPosition)
    {
        if (const auto transform = object.GetComponent<Engine::Transform>())
        {
            outPosition = transform->GetWorldPosition();
            return true;
        }

        return false;
    }
}

void Engine::Level::Load(NetCode::InputByteStream& stream)
{
    Read(stream);

    // Deliberately no game mode. A level only ever arrives over the wire on a
    // machine that is not the authority, and the game mode is the authority's --
    // it decides spawns and rules, which a client must never do for itself. The
    // JSON path builds one because that is how the authority's own level loads.
}

void Engine::Level::Start() const
{
    _gameMode->Start();
}

Engine::GameObject* Engine::Level::SpawnNewGameObject(const std::string& filepath, const glm::vec2 position)
{
    // Create Game Object
    auto go = std::make_unique<GameObject>();
    go->MarkDirty();
    if (!filepath.empty())
    {
        std::ifstream file(filepath);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file " << filepath << '\n';
            return nullptr;
        }
        
        json j;
        file >> j; // Parse JSON from the file stream
        go->Deserialize(j);
    }
    else
    {
        go->AddComponent<Transform>();
    }

    go->SetPosition(position);
    
    // Move the unique_ptr into the vector
    auto* rawPtr = go.get();
    _gameObjects.push_back(std::move(go));

    // Return the raw pointer which now safely points to the object in the vector
    return rawPtr;
}

Engine::GameObject* Engine::Level::SpawnNewGameObjectFromJson(const nlohmann::json& data)
{
    auto go = std::make_unique<GameObject>();
    go->MarkDirty();
    go->Deserialize(data);

    // Move the unique_ptr into the vector
    auto* rawPtr = go.get();
    _gameObjects.push_back(std::move(go));

    // Return the raw pointer which now safely points to the object in the vector
    return rawPtr;
}

Engine::GameObject* Engine::Level::SpawnNewGameObjectFromInputStream(NetCode::InputByteStream& stream, const uint32_t NID)
{
    auto newGo = std::make_unique<GameObject>();
    NetCode::GetLinkingContext().AddGameObject(newGo.get(), NID);
    newGo->Read(stream);
    
    auto* rawPtr = newGo.get();
    _gameObjects.push_back(std::move(newGo));
    return rawPtr;
}

bool Engine::Level::RemoveGameObject(GameObject* go, bool replicate)
{
    for (auto it = _gameObjects.begin(); it != _gameObjects.end(); )
    {
        if (it->get() == go) // Compare raw pointers
        {
            // Resolved here, while the object is still alive. The erase below runs its
            // destructor, and the old code kept the raw pointer in a list and read its
            // network id at send time -- off freed memory.
            if (const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(go, false); networkID != NULL_ID)
            {
                // Queued for each peer separately. One shared list was drained by the
                // first send of the tick, so only whichever peer happened to go first
                // ever learned the object was gone.
                if (replicate)
                    for (auto& replicationPeer : _replicationPeers)
                        if (replicationPeer.active)
                            replicationPeer.pendingDestroys.push_back(networkID);

                // Unregistered whether or not anyone is told about it. `replicate` says
                // who to notify, not whether the object still exists -- and a linking
                // context entry outliving its object hands the next lookup a pointer to
                // freed memory that reads as perfectly live.
                //
                // This was latent until area-of-interest arrived: a client only ever saw
                // a destruction when somebody disconnected, and those network ids are
                // never reissued. Now objects leave and re-enter relevance constantly,
                // and a re-entering one was deserialized straight into the corpse of its
                // previous self.
                NetCode::GetLinkingContext().RemoveGameObject(go);
            }

            // Whoever was looking from this object is not any more. Read every tick by
            // UpdateRelevance, so leaving it set is a dangling read.
            for (auto& replicationPeer : _replicationPeers)
                if (replicationPeer.pawn == go)
                    replicationPeer.pawn = nullptr;

            if(auto parent = go->GetParent())
            {
                parent->RemoveChild(go);
            }

            // Children outlive their parent here -- only this object is being removed --
            // so they must not be left holding a pointer to it.
            for (const auto child : go->GetChildren())
                if (child != nullptr)
                    child->_parent = nullptr;
            
            it = _gameObjects.erase(it); // Erase and update iterator
        }
        else
        {
            ++it;
        }
    }

    return true;
}

void Engine::Level::SaveLevel(const std::string& args)
{
    nlohmann::json data;
    
    // Update level data
    data[JsonKeywords::LEVEL_NAME] = _name;
    data["Size X"] = _size.x;
    data["Size Y"] = _size.y;

    // Update game objects
    data[JsonKeywords::GAMEOBJECT_ARRAY] = json::array();
    for (const auto& go : _gameObjects)
        if(!go->GetParent())
            data[JsonKeywords::GAMEOBJECT_ARRAY].push_back(go->Serialize());

    // Write new data
    if (std::ofstream outputFile(_path); outputFile.is_open())
    {
        outputFile << data.dump(4);
        outputFile.close();
    }

    std::cout << "Level Saved Successfully!" << '\n';
}

void Engine::Level::WriteCompleteStateFor(const NetCode::PeerID peer, NetCode::OutputByteStream& stream)
{
    const int slot = FindReplicationSlot(peer);
    const uint32_t bit = slot >= 0 ? 1u << slot : 0u;

    glm::vec2 viewpoint;
    const bool hasViewpoint = slot >= 0
        && _replicationPeers[slot].pawn != nullptr
        && TryGetWorldPosition(*_replicationPeers[slot].pawn, viewpoint);

    // Relevance applies to replicated objects only. An object with isReplicated = false
    // never enters or leaves anybody's set -- it is sent once here and never mentioned
    // again -- so filtering it by distance would mean it never arrives at all.
    const auto included = [&](const GameObject& object)
    {
        if (object.isServerOnly) return false;
        if (!hasViewpoint || !object.isReplicated) return true;
        return IsRelevantTo(object, viewpoint, false);
    };

    uint32_t replicatedCount = 0;
    for (const auto& element : _gameObjects)
        if (included(*element))
            ++replicatedCount;

    stream.Write(replicatedCount);
    for (auto& element : _gameObjects)
    {
        if (!included(*element)) continue;

        // Marked known as it is written. The snapshot goes out on a channel with an
        // empty window, so it does not face the refusal a delta does -- and
        // NetworkManager drops the peer outright if the send fails anyway.
        element->_knownToPeers |= bit;

        const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(element.get());
        stream.Write(networkID);

        // Every record carries a bit saying which of the two forms follows, so the
        // reader is one code path and a complete state is just the case where they all
        // happen to be full.
        stream.Write(true);
        element->Write(stream);
    }

    // No destruction records: a peer receiving a complete state has never heard of
    // anything already destroyed, so there is nothing to tell it to remove. Its
    // replication slot opens with an empty pending list for the same reason.
    stream.Write(static_cast<uint32_t>(0));
}

int Engine::Level::FindReplicationSlot(const NetCode::PeerID peer) const
{
    for (int i = 0; i < static_cast<int>(NetCode::MAX_REPLICATION_PEERS); ++i)
        if (_replicationPeers[i].active && _replicationPeers[i].peer == peer)
            return i;

    return -1;
}

void Engine::Level::AddReplicationPeer(const NetCode::PeerID peer, GameObject* pawn)
{
    if (FindReplicationSlot(peer) >= 0) return;

    int slot = -1;
    for (int i = 0; i < static_cast<int>(NetCode::MAX_REPLICATION_PEERS); ++i)
    {
        if (!_replicationPeers[i].active)
        {
            slot = i;
            break;
        }
    }

    if (slot < 0)
    {
        DEBUG_LOG("Level: no replication slot left for peer %llu (max %u).", peer, NetCode::MAX_REPLICATION_PEERS)
        return;
    }

    _replicationPeers[slot].peer = peer;
    _replicationPeers[slot].active = true;
    _replicationPeers[slot].pawn = pawn;
    _replicationPeers[slot].pendingDestroys.clear();
    _activePeerMask |= 1u << slot;

    // Opens knowing nothing and owed nothing. WriteCompleteStateFor marks what it
    // actually sends; everything else stays unknown until it becomes relevant. Cleared
    // explicitly rather than assumed, because a slot reused after a disconnect would
    // otherwise inherit the previous occupant's state.
    const uint32_t bit = 1u << slot;
    for (auto& element : _gameObjects)
    {
        element->_dirtyPeers &= ~bit;
        element->_knownToPeers &= ~bit;
    }
}

void Engine::Level::RemoveReplicationPeer(const NetCode::PeerID peer)
{
    const int slot = FindReplicationSlot(peer);
    if (slot < 0) return;

    _replicationPeers[slot].active = false;
    _replicationPeers[slot].peer = NetCode::INVALID_PEER;
    _replicationPeers[slot].pawn = nullptr;
    _replicationPeers[slot].pendingDestroys.clear();
    _activePeerMask &= ~(1u << slot);

    // Cleared here as well as in AddReplicationPeer. Belt and braces: an object that
    // outlives the peer must not claim the next occupant of this slot already knows it.
    const uint32_t bit = 1u << slot;
    for (auto& element : _gameObjects)
        element->_knownToPeers &= ~bit;
}

bool Engine::Level::IsRelevantTo(const GameObject& object, const glm::vec2& viewpoint, const bool currentlyKnown)
{
    glm::vec2 position;
    if (!TryGetWorldPosition(object, position))
        return true;

    const float radius = currentlyKnown ? RELEVANCE_LEAVE_RADIUS : RELEVANCE_ENTER_RADIUS;
    const glm::vec2 offset = position - viewpoint;
    const float distanceSquared = offset.x * offset.x + offset.y * offset.y;

    // Squared throughout -- there is no reason to pay for a square root per object per
    // peer per tick just to compare against a constant.
    return distanceSquared <= radius * radius;
}

void Engine::Level::UpdateRelevance()
{
    for (int slot = 0; slot < static_cast<int>(NetCode::MAX_REPLICATION_PEERS); ++slot)
    {
        ReplicationPeer& replicationPeer = _replicationPeers[slot];
        if (!replicationPeer.active) continue;

        glm::vec2 viewpoint;
        if (replicationPeer.pawn == nullptr || !TryGetWorldPosition(*replicationPeer.pawn, viewpoint))
            continue; // no viewpoint yet, so nothing to filter against; send everything

        const uint32_t bit = 1u << slot;

        for (auto& element : _gameObjects)
        {
            if (element->isServerOnly || !element->isReplicated) continue;

            const bool known = (element->_knownToPeers & bit) != 0;
            if (IsRelevantTo(*element, viewpoint, known) == known) continue;

            if (!known)
            {
                // Entering. Marking it dirty is all that is needed -- the form is chosen
                // from _knownToPeers, which is still clear, so it goes out in full.
                element->_dirtyPeers |= bit;
                continue;
            }

            // Leaving. Deliberately not RemoveGameObject: the object still exists on the
            // server and for every other peer, and only this one's copy is being dropped.
            if (const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(element.get(), false); networkID != NULL_ID)
                replicationPeer.pendingDestroys.push_back(networkID);

            element->_knownToPeers &= ~bit;
            element->_dirtyPeers &= ~bit;
        }
    }
}

void Engine::Level::BuildDeltaBlobs()
{
    UpdateRelevance();

    _deltaBlobs.clear();

    for (auto& element : _gameObjects)
    {
        // Owed to *somebody*, which is not the same as dirtied this frame: a peer whose
        // update could not be sent last tick still holds its bit, so the object turns up
        // here again and is re-serialized at its current state rather than its stale one.
        const uint32_t owed = element->_dirtyPeers & _activePeerMask;
        if (owed == 0) continue;

        // isServerOnly was only ever checked on the complete-state path, so a
        // server-only object that was also replicated leaked to clients through deltas
        // -- and arrived as an unknown network id, which spawns it.
        if (!element->isReplicated || element->isServerOnly) continue;

        DeltaBlob& blob = _deltaBlobs.emplace_back();
        blob.object = element.get();
        blob.networkID = NetCode::GetLinkingContext().GetNetworkID(element.get());

        // A peer that has never seen this object cannot read a delta of it -- there is
        // no length prefix to skip past, so it would desynchronise the whole message.
        // It gets the full record instead, which doubles as the spawn instruction.
        blob.owedFull = owed & ~element->_knownToPeers;
        blob.owedDelta = owed & element->_knownToPeers;

        if (blob.owedFull != 0) element->Write(blob.fullPayload);
        if (blob.owedDelta != 0) element->WriteDelta(blob.deltaPayload);
    }
}

bool Engine::Level::HasPendingDelta(const NetCode::PeerID peer) const
{
    const int slot = FindReplicationSlot(peer);
    if (slot < 0) return false;

    if (!_replicationPeers[slot].pendingDestroys.empty()) return true;

    const uint32_t bit = 1u << slot;
    for (const auto& blob : _deltaBlobs)
        if ((blob.owedFull | blob.owedDelta) & bit)
            return true;

    return false;
}

void Engine::Level::WriteDeltaFor(const NetCode::PeerID peer, NetCode::OutputByteStream& stream) const
{
    const int slot = FindReplicationSlot(peer);
    if (slot < 0) return;

    const uint32_t bit = 1u << slot;

    uint32_t replicatedCount = 0;
    for (const auto& blob : _deltaBlobs)
        if ((blob.owedFull | blob.owedDelta) & bit)
            ++replicatedCount;

    stream.Write(replicatedCount);
    for (const auto& blob : _deltaBlobs)
    {
        const bool full = (blob.owedFull & bit) != 0;
        if (!full && (blob.owedDelta & bit) == 0) continue;

        stream.Write(blob.networkID);
        stream.Write(full);

        // Spliced bit-exactly out of the blob rather than re-serialized. The stream is
        // bit-granular and WriteBits copies at an arbitrary destination offset, so this
        // produces the identical bytes a direct Write would have, for a memcpy.
        const NetCode::OutputByteStream& payload = full ? blob.fullPayload : blob.deltaPayload;
        stream.WriteBits(payload.GetBuffer(), payload.GetBitLength());
    }

    const std::vector<uint32_t>& destroys = _replicationPeers[slot].pendingDestroys;
    stream.Write(static_cast<uint32_t>(destroys.size()));
    for (const uint32_t networkID : destroys)
        stream.Write(networkID);
}

void Engine::Level::CommitDeltaFor(const NetCode::PeerID peer)
{
    const int slot = FindReplicationSlot(peer);
    if (slot < 0) return;

    const uint32_t bit = 1u << slot;

    // Only the objects actually written are retired -- not every object with the bit
    // set -- so anything dirtied after the blobs were built still goes out next tick.
    for (const auto& blob : _deltaBlobs)
    {
        if (((blob.owedFull | blob.owedDelta) & bit) == 0) continue;

        blob.object->_dirtyPeers &= ~bit;

        // Having received the full record, this peer can be sent deltas of it from now
        // on. Set only on commit, for the same reason the dirty bit is cleared only on
        // commit: a refused send means the peer never got the record.
        if (blob.owedFull & bit)
            blob.object->_knownToPeers |= bit;
    }

    _replicationPeers[slot].pendingDestroys.clear();
}

void Engine::Level::Read(NetCode::InputByteStream& stream)
{
    uint32_t elementCount;
    stream.Read(elementCount);
    for (int i = 0; i < (int)elementCount; i++)
    {
        uint32_t networkID;
        stream.Read(networkID);

        bool full;
        stream.Read(full);

        const auto go = NetCode::GetLinkingContext().GetGameObject(networkID);

        if (!full)
        {
            // A delta for an object we do not have cannot even be skipped -- nothing in
            // the record says how long it is -- so the rest of the message goes with it.
            if (go == nullptr)
            {
                DEBUG_LOG("Delta for unknown network id %u; the rest of this message is unreadable.", networkID)
                return;
            }

            go->ReadDelta(stream);
            continue;
        }

        if (go == nullptr)
        {
            const auto newGo = SpawnNewGameObjectFromInputStream(stream, networkID);
            DEBUG_LOG("Creating game object: %s", newGo->GetName().c_str())
        }
        else
        {
            go->Read(stream);
        }
    }

    uint32_t destroyedCount;
    stream.Read(destroyedCount);
    for (int i = 0; i < (int)destroyedCount; i++)
    {
        uint32_t networkID;
        stream.Read(networkID);

        GameObject* go = NetCode::GetLinkingContext().GetGameObject(networkID);
        if (go == nullptr)
        {
            // Worth seeing rather than silently ignoring: it means this peer was told to
            // remove something it was never sent, which is a replication bookkeeping bug.
            DEBUG_LOG("Destroy for unknown network id %u; ignoring.", networkID)
            continue;
        }

        DEBUG_LOG("Destroying game object: %s", go->GetName().c_str())
        RemoveGameObject(go, false);
    }

    for (const auto& go : _gameObjects)
    {
        go->LinkFamily();
    }
}

std::vector<Engine::GameObject*> Engine::Level::GetAllGameObjects() const
{
    std::vector<GameObject*> result;
    result.reserve(_gameObjects.size());  // Optimize memory allocation
    for (auto& obj : _gameObjects) {
        result.push_back(obj.get());  // Push raw pointers to GameObject
    }
    return result;
}
