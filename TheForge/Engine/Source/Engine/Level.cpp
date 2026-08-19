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
            if (replicate)
            {
                // Resolved here, while the object is still alive. The erase below runs
                // its destructor, and the old code kept the raw pointer in a list and
                // read its network id at send time -- off freed memory.
                if (const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(go, false); networkID != NULL_ID)
                {
                    // Queued for each peer separately. One shared list was drained by the
                    // first send of the tick, so only whichever peer happened to go first
                    // ever learned the object was gone.
                    for (auto& replicationPeer : _replicationPeers)
                        if (replicationPeer.active)
                            replicationPeer.pendingDestroys.push_back(networkID);

                    NetCode::GetLinkingContext().RemoveGameObject(go);
                }
            }

            if(auto parent = go->GetParent())
            {
                parent->RemoveChild(go);
            }
            
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

void Engine::Level::Write(NetCode::OutputByteStream& stream)
{
    uint32_t replicatedCount = 0;
    for (const auto& element : _gameObjects)
        if (!element->isServerOnly)
            ++replicatedCount;

    stream.Write(replicatedCount);
    for (auto& element : _gameObjects)
    {
        if (element->isServerOnly) continue;

        const uint32_t networkID = NetCode::GetLinkingContext().GetNetworkID(element.get());
        stream.Write(networkID);
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

void Engine::Level::AddReplicationPeer(const NetCode::PeerID peer)
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
    _replicationPeers[slot].pendingDestroys.clear();
    _activePeerMask |= 1u << slot;

    // Cleared rather than set. The peer is handed the complete world as it joins, so
    // at the moment its slot opens it is owed nothing -- and a slot reused after a
    // disconnect would otherwise inherit the previous occupant's backlog.
    const uint32_t bit = 1u << slot;
    for (auto& element : _gameObjects)
        element->_dirtyPeers &= ~bit;
}

void Engine::Level::RemoveReplicationPeer(const NetCode::PeerID peer)
{
    const int slot = FindReplicationSlot(peer);
    if (slot < 0) return;

    _replicationPeers[slot].active = false;
    _replicationPeers[slot].peer = NetCode::INVALID_PEER;
    _replicationPeers[slot].pendingDestroys.clear();
    _activePeerMask &= ~(1u << slot);
}

void Engine::Level::BuildDeltaBlobs()
{
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
        blob.owedTo = owed;
        element->Write(blob.payload);
    }
}

bool Engine::Level::HasPendingDelta(const NetCode::PeerID peer) const
{
    const int slot = FindReplicationSlot(peer);
    if (slot < 0) return false;

    if (!_replicationPeers[slot].pendingDestroys.empty()) return true;

    const uint32_t bit = 1u << slot;
    for (const auto& blob : _deltaBlobs)
        if (blob.owedTo & bit)
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
        if (blob.owedTo & bit)
            ++replicatedCount;

    stream.Write(replicatedCount);
    for (const auto& blob : _deltaBlobs)
    {
        if ((blob.owedTo & bit) == 0) continue;

        stream.Write(blob.networkID);

        // Spliced bit-exactly out of the blob rather than re-serialized. The stream is
        // bit-granular and WriteBits copies at an arbitrary destination offset, so this
        // produces the identical bytes a direct Write would have, for a memcpy.
        stream.WriteBits(blob.payload.GetBuffer(), blob.payload.GetBitLength());
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
        if (blob.owedTo & bit)
            blob.object->_dirtyPeers &= ~bit;

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
        if (const auto go = NetCode::GetLinkingContext().GetGameObject(networkID); go == nullptr)
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
