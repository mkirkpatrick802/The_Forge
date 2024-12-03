#include "Engine/System.h"

int main()
{
    Engine::System::Init();

    {
        printf("Hello World\n");   
    }
    
    Engine::System::CleanUp();
}