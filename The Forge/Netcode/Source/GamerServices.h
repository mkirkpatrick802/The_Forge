#pragma once
#include <map>
#include <memory>
#include <string>

namespace NetCode
{
    class OutputByteStream;
    class GamerServices
    {
    public:
        static void Init();
        static void CleanUp();
        static std::unique_ptr<GamerServices> instance;

        // General player functions
        uint64_t GetLocalPlayerID() const;
        std::string GetLocalPlayerName() const;
        std::string GetRemotePlayerName(uint64_t inPlayerID) const;

        // Lobby functions
        void LobbySearchAsync();
        int GetLobbyNumPlayers(uint64_t inLobbyID) const;
        uint64_t GetOwnerID(uint64_t inLobbyID) const;
        void GetLobbyPlayerMap(uint64_t inLobbyID, std::map<uint64_t, std::string>& outPlayerMap) const;
        void LeaveLobby(uint64_t inLobbyID);

        // P2P networking
        bool SendP2PReliable( const OutputByteStream& inOutputStream, uint64_t inToPlayer );
        bool IsP2PPacketAvailable( uint32_t& outPacketSize );
        uint32_t ReadP2PPacket( void* inToReceive, uint32_t inMaxLength, uint64_t& outFromPlayer );
    
    public:
        ~GamerServices();
        void Update() const;
        struct Impl;

    private:
        GamerServices();
        std::unique_ptr<Impl> _impl;
    };

    inline GamerServices& GetGamerService()
    {
        return *GamerServices::instance;
    }
}
