#include <Gorgon/Scene.h>
#include "GameScene.h"
#include <Gorgon/UI.h>

#include <cstdlib>
#include <ctime>

int main(int argc, char *argv[]) {
    Gorgon::Initialize("GDTD");
    
    Gorgon::SceneManager scenemanager(Gorgon::SceneManager::Fullscreen, "GDTD");
    Gorgon::UI::Initialize();
    
    int seed = -1;
    
    if(argc > 1) {
        seed = atoi(argv[1]);
    }
    
    scenemanager.NewScene<Game>(GameScene, seed);
    scenemanager.SwitchScene(GameScene);
    
    scenemanager.Run();

    return 0;
}
