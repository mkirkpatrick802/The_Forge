#include <steam/steamnetworkingsockets.h>
#include <glm.hpp>
#include <nlohmann/json.hpp>

int main(void) {
    SteamNetworkingIdentity identity;

    identity.ParseString("str:peer-server");

    glm::vec3 yo = glm::vec3(1);

    return 0;
}