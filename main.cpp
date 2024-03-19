#include <steam/steamnetworkingsockets.h>

int main(void) {
    SteamNetworkingIdentity identity;

    identity.ParseString("str:peer-server");

    return 0;
}