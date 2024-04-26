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
        System::IsEditor = false;

        NetCode::Init();

        bool isServer;

        printf("Would You Like to Host a Game? \n");
        std::cin >> isServer;

        if (isServer)
            netcode = new Server();
        else
            netcode = new Client();

        // Start Client
        if(!isServer)
        {
	        std::string address;
	        printf("What IP Address Would You Like To Join? \n");
            std::cin >> address;

            if (address == "localhost")
                address = "127.0.0.1";

            netcode->SetIPv4Address(address);
            netcode->Start();

            while (!dynamic_cast<Client*>(netcode)->isConnected)
            {
                netcode->Update(0);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            return;
        }

        // Start Server
    	netcode->Start();
    }
}

void GameplayLoop()
{
    renderer.CreateRenderer();
    UIManager::Init(renderer);

    if(!buildPlayMode)
    {
        EditorEngine engine = EditorEngine(renderer, inputManager);
        engine.GameLoop();
        engine.CleanUp();
    }
    else
    {
        PlayEngine engine = PlayEngine(renderer, inputManager, netcode);
    	engine.GameLoop();
        engine.CleanUp();
    }
}