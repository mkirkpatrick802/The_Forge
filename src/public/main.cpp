#define SDL_MAIN_HANDLED true

#include "Renderer.h"
#include "UIManager.h"
#include "System.h"
#include "InputManager.h"
#include "PlayEngine.h"
#include "EditorEngine.h"

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

    GameplayLoop();

    inputManager.CleanUp();
    renderer.CleanUp();
    system.CleanUp();
}

/*NetCode::Init();
if (const auto client = dynamic_cast<Client*>(netcode))
{
    printf("Connecting To Server... \n");
    while (!client->isConnected)
    {
        client->Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}*/

/*void StartingGameOptions()
{

    printf("Build Play Mode? \n");
    std::cin >> buildPlayMode;

    if(buildPlayMode) 
    {
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
    }
}*/

void GameplayLoop()
{
    if(!buildPlayMode)
    {
        printf("Entering Editor.. \n");

        renderer.CreateRenderer();

        UIManager uiManager = UIManager(renderer);
        EditorEngine engine = EditorEngine(renderer, inputManager, uiManager);
        engine.GameLoop();

        engine.CleanUp();
        uiManager.CleanUp();
    }
    else
    {
        printf("Building Play Mode... \n");

        renderer.CreateRenderer();
        PlayEngine engine = PlayEngine(renderer, inputManager, netcode);
        engine.GameLoop();

        engine.CleanUp();
    }
}