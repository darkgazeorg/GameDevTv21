#include <Gorgon/Scene.h>
#include "GameScene.h"
#include <Gorgon/UI.h>

#include <cstdlib>
#include <ctime>

int main() {
    std::srand(std::time(nullptr));

    Gorgon::Initialize("GDTD");
    
    Gorgon::SceneManager scenemanager(Gorgon::SceneManager::Fullscreen, "GDTD");
    Gorgon::UI::Initialize();
    
    scenemanager.NewScene<Game>(GameScene);
    scenemanager.SwitchScene(GameScene);
    
    scenemanager.Run();

    return 0;
}
