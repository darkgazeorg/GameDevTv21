#include <Gorgon/Scene.h>
#include "GameScene.h"
#include <Gorgon/UI.h>

int main() {
    Gorgon::Initialize("GDTD");
    
    Gorgon::SceneManager scenemanager({1000, 800}, "GDTD");
    Gorgon::UI::Initialize();
    
    scenemanager.NewScene<Game>(GameScene);
    scenemanager.SwitchScene(GameScene);
    
    scenemanager.Run();

    return 0;
}
