#include "NetObject.h"
#include <steam/steam_api_common.h>


void NetCode::NetObject::Update()
{
    SteamAPI_RunCallbacks();
}