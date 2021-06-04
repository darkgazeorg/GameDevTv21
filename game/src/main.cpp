#include <Gorgon/Scene.h>
#include "GameScene.h"
#include <Gorgon/UI.h>

#include <cstdlib>
#include <ctime>

#include "Types.h"
#include "MainMenu.h"
#include "HowTo.h"
#include "Credits.h"
#include "EndGame.h"

std::default_random_engine RNG;
Game *Game::inst;

int main(int argc, char *argv[]) {
    Gorgon::Initialize("GDTD");
    
    Gorgon::SceneManager scenemanager(Gorgon::SceneManager::Fullscreen, "GDTD");
    Gorgon::UI::Initialize();

    
    scenemanager.NewScene<MainMenu>(MenuScene);
    scenemanager.NewScene<Game>(GameScene);
    scenemanager.NewScene<HowTo>(HowToScene);
    scenemanager.NewScene<Credits>(CreditsScene);
    scenemanager.NewScene<EndGame>(EndGameScene);
    
    scenemanager.SwitchScene(MenuScene);
    
    scenemanager.Run();

    return 0;
}
