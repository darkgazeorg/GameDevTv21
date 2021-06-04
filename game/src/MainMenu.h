#pragma once

#include "Scenes.h"
#include <Gorgon/Scene.h>
#include "Types.h"
#include <Gorgon/Widgets/Button.h>
#include <Gorgon/Widgets/Label.h>
#include <Gorgon/Widgets/Layerbox.h>
#include <Gorgon/UI/Dialog.h>
#include "Resources.h"
#include "GameScene.h"

#define rint(min, max)    std::uniform_int_distribution<int>(min, max)(RNG)

class MainMenu : public Gorgon::Scene {
public:
    MainMenu(Gorgon::SceneManager &parent, Gorgon::SceneID id) :
        Scene(parent, id)
    {
        ui.Add(nw);
        nw.Text = "New Game";
        nw.Move(170, 470);
        nw.Resize(216, ui.GetUnitWidth()*1.5);
        nw.ClickEvent.Register([&] {
            if(Gorgon::Input::Keyboard::CurrentModifier.IsModified()) {
                Gorgon::UI::Input<int>(
                    "New game", 
                    "Set the seed for the new game, use -1 for random seed.", 
                    "Seed", [&](int v) {
                        Game::Get().Reset(v);
                        parent.SwitchScene(GameScene);
                    }, -1, {}, Gorgon::UI::CloseOption::Close
                );
            }
            else {
                Game::Get().Reset(-1);
                parent.SwitchScene(GameScene);
            }
        });
        
        ui.Add(howto);
        howto.Text = "How to play";
        howto.Move(170, 470 + ui.GetUnitWidth()*2);
        howto.Resize(216, ui.GetUnitWidth()*1.5);
        howto.ClickEvent.Register([&] {
            parent.SwitchScene(HowToScene);
        });
        
        ui.Add(credits);
        credits.Text = "Credits";
        credits.Move(170, 470 + ui.GetUnitWidth()*4);
        credits.Resize(216, ui.GetUnitWidth()*1.5);
        credits.ClickEvent.Register([&] {
            parent.SwitchScene(CreditsScene);
        });
        
        ui.Add(quit);
        quit.Text = "Exit game";
        quit.Move(170, 470 + ui.GetUnitWidth()*6);
        quit.Resize(216, ui.GetUnitWidth()*1.5);
        quit.ClickEvent.Register([&] {
            parent.Quit();
        });
    }
    
    virtual bool RequiresKeyInput() const override {
        return true;
    }
    
    virtual void render() override {
    }
    
    void doframe(unsigned int delta) override {
    }
    
    virtual void activate() override {
        graphics.Clear();
        resources.Root().Get<R::Folder>(2).Get<R::Image>("BG").DrawIn(graphics);
        resources.Root().Get<R::Folder>(2).Get<R::Image>("mm").DrawIn(graphics);
    }
    
private:
    
    Widgets::Button nw, credits, howto, quit;
};
