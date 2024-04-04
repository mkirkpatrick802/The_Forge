#define SDL_MAIN_HANDLED true

#include <iostream>
#include <thread>

#include "Renderer.h"
#include "System.h"
#include "InputManager.h"
#include "PlayEngine.h"
#include "EditorEngine.h"
#include "Server.h"
#include "Client.h"
#include "UIManager.h"

// Managers
Renderer renderer;
InputManager inputManager;

// Gates
bool buildPlayMode = false;

void StartingGameOptions();
void GameplayLoop();

NetCode* netcode;

int main(int argc, char* argv[]) {

    System system = System();

    renderer = Renderer();
    inputManager = InputManager();

    StartingGameOptions();
    GameplayLoop();

    inputManager.CleanUp();
    UIManager::CleanUp();
    renderer.CleanUp();
    system.CleanUp();
}

void StartingGameOptions()
{

    printf("Build Play Mode? \n");
    std::cin >> buildPlayMode;

    if(buildPlayMode) 
    {
        NetCode::Init();

        bool isServer;

        printf("Would You Like to Host a Game? \n");
        std::cin >> isServer;

        if (isServer)
            netcode = new Server();
        else
            netcode = new Client();

        if(!isServer)
        {
	        std::string address;
	        printf("What IP Address Would You Like To Join? \n");
            std::cin >> address;

            if (address == "localhost")
                address = "127.0.0.1";

            netcode->SetIPv4Address(address);
        }

        netcode->Start();

        if (const auto client = dynamic_cast<Client*>(netcode))
        {
            printf("Connecting To Server... \n");
            while (!client->isConnected)
            {
                client->Update();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
}

void GameplayLoop()
{
    renderer.CreateRenderer();
    UIManager::Init(renderer);

    if(!buildPlayMode)
    {
        printf("Entering Editor.. \n");

        EditorEngine engine = EditorEngine(renderer, inputManager);
        engine.GameLoop();
    }
    else
    {
        printf("Building Play Mode... \n");

        PlayEngine engine = PlayEngine(renderer, inputManager, netcode);
        engine.GameLoop();
    }
}